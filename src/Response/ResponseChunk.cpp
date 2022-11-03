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

// GET --> is CGI ? CGI : FILE
// POST --> CGI
// DELETE --> unlink

void Response::processRequest() {
	initialize();

	if (m_statusCode == 200) {
		switch (m_request.getMethod()) {
			case GET:
				m_isCGI ? handleCGI() : handleFile();
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

	if (m_statusCode != 200 && m_statusCode != 201 && m_statusCode != 301) // TODO: ugly
		serveError(m_request.getErrorMsg());

	if (!m_isCGI)
		m_chunk = getResponseAsString();
}

void Response::handleDelete() {
	std::string absolute = getRealPath(m_filename);

	LOG(RED "Handle Delete: " DEFAULT + m_filename);

	if (absolute.empty()) {
		m_statusCode = 404;
		return;
	}

	m_doneReading = true;

	if (unlink(absolute.c_str()) == -1) {
		LOG_ERR("unlink: " << absolute << ": " << strerror(errno));
		if (errno == EACCES)
			m_statusCode = 403;
		else
			m_statusCode = 404;
	}

	m_statusCode = 204;
}

void Response::handleFile() {
	bool isDirectory = isDir(m_filename);

	LOG(RED "Get: File: " DEFAULT + m_filename);

	if (isDirectory) {
		if (m_filename.back() != '/')
			// return sendMoved(m_request.getLocation() + "/");
			m_filename += "/";
		m_filename += "index.html"; // TODO: get from server
	}

	m_source_fd = open(m_filename.c_str(), O_RDONLY);
	if (m_source_fd == -1)
		return openError(isDirectory);

	addFileHeaders();
}

void Response::openError(bool isDirectory) {
	bool autoIndex = m_server->getAutoIndex();

	perror("open"); // TODO
	switch (errno) {
		case EACCES:
			m_statusCode = 403;
			break;
		case ENOENT:
		case ENOTDIR:
			m_statusCode = 404;
			if (isDirectory && autoIndex)
				createIndex(m_filename.substr(0, m_filename.find("index.html")));
			break;
		default:
			m_statusCode = 500;
	}
}

void Response::addFileHeaders() {
	off_t size = fileSize(m_source_fd);

	addHeader("Content-Type", MIME::fromFileName(m_filename));

	m_isChunked = (size > BUFFER_SIZE);

	if (m_isChunked)
		addHeader("Transfer-Encoding", "Chunked");
	else
		addHeader("Content-Length", toString(size));
}

void Response::appendBodyPiece() {
	if (m_isCGI)
		writeToCGI();
	else
		m_request.getBody().clear();
}

// this function removes bytes_sent amount of bytes from the beginning of the chunk.
void Response::trimChunk(ssize_t bytes_sent) {
	m_chunk.erase(0, bytes_sent);
}

bool Response::isDone() const {
	return m_doneReading && m_chunk.empty();
}

std::string& Response::getNextChunk() {
	std::string block;

	if (!m_doneReading && m_chunk.size() < BUFFER_SIZE) {
		if (m_isCGI && !m_CGI_DoneProcessingHeaders)
			getCGIHeaderChunk();
		else {
			block = readBlockFromFile();
			if (m_isChunked)
				encodeChunked(block);
			m_chunk += block;
		}
	}
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
	addToBody(autoIndexHtml(path_to_index, m_server->getRoot(m_locationIndex)));

	m_doneReading = true;
	m_statusCode  = 200;
}
