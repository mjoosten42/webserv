#include "AutoIndex.hpp"
#include "MIME.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "buffer.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <fcntl.h> // open
#include <sys/socket.h>
#include <unistd.h> // lseek

void Response::processRequest() {
	setFlags();
	addDefaultHeaders();

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
}

void Response::handleGet() {
	LOG(RED "Handle Get" DEFAULT);

	if (m_isCGI)
		handleGetCGI();
	else
		handleGetWithFile();

	if (m_statusCode != 200)
		serveError(getStatusMessage());
	// sendFail(m_statusCode, m_isCGI ? "CGI BROKE 😂😂😂" : "Page is venting")
}

// TODO: send to CGI
void Response::handlePost() {

	if (m_isCGI) {
		handlePostCGI();
	} else {
		std::string filename = m_server->getRoot() + m_request.getLocation();

		LOG(RED "Handle Post: " DEFAULT + filename);

		addHeader("Location", m_request.getLocation());
		m_statusCode  = 418;
		m_doneReading = true;
	}
}

void Response::handleDelete() {
	std::string filename = m_server->getRoot() + m_request.getLocation();

	LOG(RED "Handle Delete: " DEFAULT + filename);

	if (unlink(filename.c_str()) == -1) {
		LOG_ERR("Unlink: " << filename << ": " << strerror(errno));
		if (errno == EACCES)
			m_statusCode = 403;
		else
			m_statusCode = 404;
	}
}

// TODO: Fix this. I added an ugly hacky param in case the file served is supposed ot be an error page.
void Response::handleGetWithFile(std::string file) {
	std::string filename		 = m_server->getRoot() + m_request.getLocation();
	bool		autoIndexInstead = false;
	if (!file.empty())
		filename = file;

	if (filename.back() == '/') {
		filename += "index.html"; // TODO: get from config. This may also be a .php file for example.
		autoIndexInstead = m_server->getAutoIndex();
	}

	LOG("Get: File: " + filename);

	m_readfd = open(filename.c_str(), O_RDONLY);
	if (m_readfd == -1) {
		if (errno == EACCES) // TODO: check if this is allowed? Subject says something about checking errno, not sure if
							 // it applies here?
			// M: We're not allowed to read/write without poll because EWOULDBLOCK/EAGAIN, but this is fine
			m_statusCode = 403;
		else if (autoIndexInstead)
			m_statusCode = autoIndex(filename.substr(0, filename.find("index.html")));
		else
			m_statusCode = 404;
	}

	addHeader("Content-Type", MIME::fromFileName(filename));

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
		ssize_t bytes_written = write(m_cgi.popen.writefd, str.c_str(), str.length());
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
		ssize_t bytes_written = write(m_cgi.popen.writefd, m_request.getBody().c_str(), m_request.getBody().length());
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

// this function removes bytesSent amount of bytes from the beginning of the chunk.
void Response::trimChunk(ssize_t bytesSent) {
	m_chunk.erase(0, bytesSent);
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

	m_isSmallFile = (size < BUFFER_SIZE);

	if (m_isSmallFile)
		addHeader("Content-Length", toString(size));
	else
		addHeader("Transfer-Encoding", "Chunked");
}

void Response::getCGIHeaderChunk() {

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
	if (!m_isSmallFile)
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

int Response::autoIndex(std::string path_to_index) {
	addHeader("Content-Type", "text/html");
	addToBody(autoIndexHtml(path_to_index, m_server->getRoot()));

	m_doneReading = true;
	return 200;
}
