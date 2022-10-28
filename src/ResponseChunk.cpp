#include "AutoIndex.hpp"
#include "MIME.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "buffer.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <fcntl.h>	// open
#include <stdlib.h> // realpath
#include <sys/socket.h>
#include <unistd.h> // lseek

void Response::processRequest() {
	initialize();

	if (m_statusCode == 200) {
		switch (m_request.getMethod()) {
			case GET:
				handleGet();
				break;
			case POST:
				handlePost();
				break;
			case DELETE:
				handleDelete();
				break;
			default:
				LOG_ERR("Invalid method");
		}
	} else
		serveError(m_request.getErrorMsg());

	if (m_statusCode != 200) {
		serveError(getStatusMessage());
	}

	if (!m_isCGI)
		m_chunk = getResponseAsString();
}

// All bools are initialized to false
// Set to true if needed
void Response::setFlags() {
	m_processedRequest = true;
	if (m_request.hasHeader("Connection"))
		if (m_request.getHeaderValue("Connection") == "close")
			m_close = true;
	m_isCGI = (MIME::getExtension(m_request.getLocation()) == "php"); // TODO: server
	m_isChunked |= m_isCGI;
}

void Response::handleGet() {
	LOG(RED "Handle Get" DEFAULT);

	if (m_isCGI)
		handleGetCGI();
	else
		handleGetWithFile();

}

// TODO: send to CGI
void Response::handlePost() {
	LOG(RED "Handle Post: " DEFAULT + m_filename);

	if (m_isCGI) {
		handlePostCGI();
	} else {
		addHeader("Location", m_request.getLocation());
		m_statusCode  = 418;
		m_doneReading = true;
	}
}

void Response::handleDelete() {
	char absolute_path[PATH_MAX + 1];

	m_doneReading = true;

	LOG(RED "Handle Delete: " DEFAULT + m_filename);

	if (!realpath(m_filename.c_str(), absolute_path)) {
		LOG_ERR("realpath: " << m_filename << ": " << strerror(errno));
		m_statusCode = 404;
		return;
	}

	LOG("Absolute path: " << absolute_path);

	if (unlink(absolute_path) == -1) {
		LOG_ERR("unlink: " << absolute_path << ": " << strerror(errno));
		if (errno == EACCES)
			m_statusCode = 403;
		else
			m_statusCode = 404;
	}

	m_statusCode = 204;
}

void Response::handleGetWithFile() {
	bool autoIndex	 = m_server->getAutoIndex();
	bool isDirectory = m_filename.back() == '/';

	LOG("Get: File: " + m_filename);

	if (isDirectory)
		m_filename += "index.html"; // TODO: get from server

	m_readfd = open(m_filename.c_str(), O_RDONLY);
	if (m_readfd == -1) {
		if (errno == EACCES)
			m_statusCode = 403;
		else if (isDirectory && autoIndex)
			createIndex(m_filename.substr(0, m_filename.find("index.html")));
		else if (errno == ENOENT) {
			m_statusCode = 404;
		} else
			m_statusCode = 500;
		return;
	}

	addHeader("Content-Type", MIME::fromFileName(m_filename));

	getFirstChunk();
}

void Response::handleGetCGI() {

	startCGIGeneric();
	if (m_statusCode == 200)
		close(m_cgi.popen.writefd); // close the writing pipe, we are not doing anything with it
}

void Response::startCGIGeneric() {
	LOG(RED "Handle CGI" DEFAULT);

	std::string filename = m_server->getRoot() + m_request.getLocation();

	// TODO: parse from config
	m_statusCode = m_cgi.start(m_request, m_server, "/usr/bin/php", filename);

	if (m_statusCode == 200) {
		addHeader("Transfer-Encoding", "Chunked");
		m_readfd					= m_cgi.popen.readfd;
		m_CGI_DoneProcessingHeaders = false;

		m_chunk = getStatusLine() + getHeadersAsString();
	}
}

void Response::appendBodyPiece(const std::string& str) {
	// TODO: non-CGI
	if (m_request.getMethod() != POST || !m_isCGI) {
		ssize_t bytes_written = write(m_cgi.popen.writefd, str.data(), str.length());
		if (bytes_written == -1) {
			// TODO
			perror("write");
		} else if (bytes_written != static_cast<ssize_t>(str.length())) {
			// TODO
			LOG_ERR("bytes written appendbodypiece != str.length");
		}
		if (m_request.getState() == DONE && m_request.getBody().empty())
			close(m_cgi.popen.writefd);
	}
}

void Response::handlePostCGI() {

	startCGIGeneric();
	if (m_statusCode == 200) {
		ssize_t bytes_written = write(m_cgi.popen.writefd, m_request.getBody().data(), m_request.getBody().length());
		if (bytes_written == -1) {
			// TODO
			perror("write");
		} else if (bytes_written != static_cast<ssize_t>(m_request.getBody().length())) {
			// TODO
			LOG_ERR("bytes written appendbodypiece != str.length");
		}
		close(m_cgi.popen.writefd);
	}
}

// this function removes bytes_sent amount of bytes from the beginning of the chunk.
void Response::trimChunk(ssize_t bytes_sent) {
	m_chunk.erase(0, bytes_sent);
}

bool Response::isDone() const {
	return m_doneReading && m_chunk.empty();
}

// gets the first chunk of a static file
void Response::getFirstChunk() {
	off_t size = lseek(m_readfd, 0, SEEK_END);

	// get file size
	if (size == -1)
		size = std::numeric_limits<off_t>().max();
	lseek(m_readfd, 0, SEEK_SET); // set back to start

	m_isChunked = (size > BUFFER_SIZE);

	if (m_isChunked)
		addHeader("Transfer-Encoding", "Chunked");
	else
		addHeader("Content-Length", toString(size));
}

void Response::getCGIHeaderChunk() {

	// TODO: a bit hacky. Should this be done just the first time? also, it works for perl, but not PHP.
	// clean up next monday.
	if (m_cgi.didExit() > 0) {
		close(m_cgi.popen.readfd);
		close(m_cgi.popen.writefd);

		m_statusCode = 502;
		m_chunk.clear();
		m_headers.clear();
		addDefaultHeaders();
		m_body.clear();
		sendFail(502, "CGI BROKE 😂😂😂");
		m_chunk = getResponseAsString();
		return;
	}

	std::string block = readBlockFromFile();

	size_t pos = m_headerEndFinder.find(block);
	if (pos == std::string::npos) {
		m_chunk += block;
	} else {
		m_CGI_DoneProcessingHeaders = true;
		std::string headers			= block.substr(0, pos);
		block						= block.substr(pos);
		encodeChunked(block);
		m_chunk += headers + block;
	}
}

std::string& Response::getNextChunk() {

	if (m_doneReading || m_chunk.size() > BUFFER_SIZE)
		return m_chunk;

	if (m_isCGI && !m_CGI_DoneProcessingHeaders) {
		getCGIHeaderChunk();
		return m_chunk;
	}

	std::string block = readBlockFromFile();
	if (m_isChunked)
		encodeChunked(block);
	m_chunk += block;
	return m_chunk;
}

void Response::encodeChunked(std::string& str) {
	str.insert(0, toHex(str.length()) + CRLF);
	str += CRLF;
}

std::string Response::readBlockFromFile() {
	std::string block;
	ssize_t		bytes_read = read(m_readfd, buf, BUFFER_SIZE - m_chunk.size());

	// LOG(RED << std::string(winSize(), '-') << DEFAULT);
	// LOG(std::string(buf, bytes_read));
	// LOG(RED << std::string(winSize(), '-') << DEFAULT);

	LOG(RED "Read: " DEFAULT << bytes_read);
	switch (bytes_read) {
		case -1:
			perror("read");
		case 0:
			m_doneReading = true;
			close(m_readfd);
			break;
		default:
			block.append(buf, bytes_read);
	}
	return block;
}

void Response::createIndex(std::string path_to_index) {
	addHeader("Content-Type", "text/html");
	addToBody(autoIndexHtml(path_to_index, m_server->getRoot()));

	m_doneReading = true;
	m_statusCode  = 200;
}
