#include "AutoIndex.hpp"
#include "MIME.hpp" // TODO: remove
#include "Response.hpp"
#include "Server.hpp"
#include "buffer.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <fcntl.h> // open
#include <sys/socket.h>

// GET --> is CGI ? CGI : FILE
// POST --> CGI
// DELETE --> unlink

void Response::processRequest() {
	m_processedRequest = true;

	initialize();

	if (!isGood(m_request.getStatus()))
		return serveError(m_request.getErrorMsg());

	try {
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
	} catch (int error) {
		m_statusCode = error;
		serveError(getStatusMessage());
	}
}

void Response::handleDelete() {
	LOG(RED "Handle Delete: " DEFAULT + m_filename);

	std::string absolute = getRealPath(m_filename);

	m_doneReading = true;

	if (unlink(absolute.c_str()) == -1) {
		LOG_ERR("unlink: " << strerror(errno) << ": " << absolute);
		if (errno == EACCES)
			throw 403;
		else
			throw 404;
	}

	m_statusCode = 204;
	m_chunk		 = getResponseAsString();
}

void Response::handleFile() {
	std::string originalFile = m_filename;
	bool		isDirectory	 = isDir(m_filename);

	LOG(RED "Get: File: " DEFAULT + m_filename);

	if (isDirectory) {
		if (m_filename.back() != '/')
			return sendMoved(m_request.getLocation() + "/");
		m_filename += "index.html"; // TODO: get from server
	}

	m_source_fd = open(m_filename.c_str(), O_RDONLY);
	if (m_source_fd == -1)
		return openError(originalFile, isDirectory);

	addFileHeaders();
	m_statusCode = 200;
	m_chunk		 = getResponseAsString();
}

void Response::openError(const std::string& dir, bool isDirectory) {
	bool autoIndex = m_server->getAutoIndex();

	LOG_ERR("open: " << strerror(errno) << ": " << m_filename);
	switch (errno) {
		case EACCES:
			throw 403;
		case ENOENT:
		case ENOTDIR:
			if (isDirectory && autoIndex)
				return createIndex(dir);
			throw 404;
		default:
			throw 500;
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
	addToBody(autoIndexHtml(path_to_index));

	m_doneReading = true;
	m_statusCode  = 200;

	m_chunk = getResponseAsString();
}
