#include "AutoIndex.hpp"
#include "MIME.hpp" // TODO: remove
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

// GET --> CGI ? CGI : FILE
// POST --> CGI
// DELETE --> unlink

void Response::processRequest() {
	initialize();

	if (m_statusCode == 200) {
		switch (m_request.getMethod()) {
			case GET:
				m_isCGI ? handleCGI() : handleGetWithFile();
				break;
			case POST:
				handleCGI();
				break;
			case DELETE:
				handleDelete();
				break;
			default:
				LOG_ERR("Invalid method");
		}
	}

	if (m_statusCode != 200)
		serveError(m_request.getErrorMsg());

	if (!m_isCGI)
		m_chunk = getResponseAsString();
}

void Response::handleDelete() {
	std::string absolute = getRealPath(m_filename);

	LOG(RED "Handle Delete: " DEFAULT + m_filename);

	m_doneReading = true;

	if (absolute.empty()) {
		m_statusCode = 404;
		return;
	}

	if (unlink(absolute.c_str()) == -1) {
		LOG_ERR("unlink: " << absolute << ": " << strerror(errno));
		if (errno == EACCES)
			m_statusCode = 403;
		else
			m_statusCode = 404;
	}

	m_statusCode = 204;
}

// TODO: check if directory (curl localhost:8080/img loops forever)
void Response::handleGetWithFile() {
	bool autoIndex	 = m_server->getAutoIndex();
	bool isDirectory = m_filename.back() == '/';

	LOG("Get: File: " + m_filename);

	if (isDirectory)
		m_filename += "index.html"; // TODO: get from server

	m_source_fd = open(m_filename.c_str(), O_RDONLY);
	if (m_source_fd == -1) {
		if (errno == EACCES)
			m_statusCode = 403;
		else if (isDirectory && autoIndex)
			createIndex(m_filename.substr(0, m_filename.find("index.html")));
		else if (errno == ENOENT)
			m_statusCode = 404;
		else
			m_statusCode = 500;
		return;
	}

	addHeader("Content-Type", MIME::fromFileName(m_filename));

	getFirstChunk();
}

void Response::handleCGI() {
	LOG(RED "Handle CGI: " DEFAULT + m_filename);

	// TODO: parse from config
	std::string bin = "/usr/bin/php";
	if (MIME::getExtension(m_request.getLocation()) == "pl")
		bin = "/usr/bin/pl";

	m_statusCode = m_cgi.start(m_request, m_server, bin, m_filename);

	if (m_statusCode == 200) {
		addHeader("Transfer-Encoding", "Chunked");
		m_source_fd					= m_cgi.popen.readfd;
		m_CGI_DoneProcessingHeaders = false;

		m_chunk = getStatusLine() + getHeadersAsString();
	} else {
	} // TODO
}

void Response::appendBodyPiece() {
	if (m_isCGI)
		writeToCGI();
	else
		m_request.getBody().clear();
}

void Response::writeToCGI() {
	std::string& body		   = m_request.getBody();
	ssize_t		 bytes_written = write(m_cgi.popen.writefd, body.data(), body.length());

	LOG(RED "Write: " DEFAULT << bytes_written);

	if (bytes_written == -1)
		fatal_perror("write"); // TODO
	body.erase(0, bytes_written);

	if (m_request.getState() == DONE && body.empty())
		close(m_cgi.popen.writefd);
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
	off_t size = lseek(m_source_fd, 0, SEEK_END);

	// get file size
	if (size == -1)
		size = std::numeric_limits<off_t>().max();
	lseek(m_source_fd, 0, SEEK_SET); // set back to start

	m_isChunked = (size > BUFFER_SIZE);

	if (m_isChunked)
		addHeader("Transfer-Encoding", "Chunked");
	else
		addHeader("Content-Length", toString(size));
}

void Response::getCGIHeaderChunk() {

	std::string block = readBlockFromFile();

	size_t pos = findHeaderEnd(block);
	if (pos == std::string::npos) {
		m_chunk += block;
	} else {
		m_CGI_DoneProcessingHeaders = true;
		std::string headers			= block.substr(0, pos);
		block						= block.substr(pos);

		if (block.empty()) // Only send one trailing chunk
			m_doneReading = true;

		encodeChunked(block);
		m_chunk += headers + block;
	}
}

size_t Response::findHeaderEnd(const std::string str) {
	size_t pos			 = findNewline(str);
	size_t newlineLength = (str[pos] == '\r' ? 2 : 1); // "\r\n or \n"
	size_t second;

	if (m_chunkEndedWithNewline && pos == 0)
		return newlineLength;

	while (pos < str.size()) {
		second = findNewline(str, pos + newlineLength);
		if (second == pos + newlineLength)
			return pos + newlineLength + 1;
		pos			  = second;
		newlineLength = (str[pos] == '\r' ? 2 : 1);
	}

	m_chunkEndedWithNewline = (str.back() == '\n');

	return pos;
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
	ssize_t		bytes_read = read(m_source_fd, buf, BUFFER_SIZE - m_chunk.size());

	LOG(RED "Read: " DEFAULT << bytes_read);
	switch (bytes_read) {
		case -1:
			perror("read");
		case 0:
			m_doneReading = true;
			close(m_source_fd);
			break;
		default:
			// LOG(RED << std::string(winSize(), '-') << DEFAULT);
			// LOG(std::string(buf, bytes_read));
			// LOG(RED << std::string(winSize(), '-') << DEFAULT);

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
