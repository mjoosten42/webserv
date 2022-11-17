#include "AutoIndex.hpp"
#include "MIME.hpp" // TODO: remove
#include "Response.hpp"
#include "Server.hpp"
#include "buffer.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "syscalls.hpp"
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
		return sendFail(m_request.getStatus(), m_request.getErrorMsg());

	if (m_request.getContentLength() > m_server->getCMB())
		return sendFail(413, "Max body size is " + toString(m_server->getCMB()));

	if (m_server->isRedirect(m_locationIndex))
		return sendMoved(m_server->getRedirect(m_locationIndex));

	// Limit_except is now implemented within the functions called in this try-catch block.
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
		m_status = error;
		sendFail(error, getStatusMessage());
	}
}

void Response::handleDelete() {
	if (!m_server->hasMethod(m_locationIndex, m_request.getMethod()))
		throw 405;
	std::string absolute = WS::realpath(m_filename);

	m_doneReading = true;

	if (unlink(absolute.c_str()) == -1) {
		LOG_ERR("unlink: " << strerror(errno) << ": " << absolute);
		if (errno == EACCES)
			throw 403;
		else
			throw 404;
	}

	m_status = 204;
	m_chunk	 = getResponseAsString();
}

void Response::handleFile() {
	if (!m_server->hasMethod(m_locationIndex, m_request.getMethod()))
		throw 405;
	std::string originalFile = m_filename;
	bool		isDirectory	 = isDir(m_filename);

	if (isDirectory) {
		if (m_filename.back() != '/')
			return sendMoved(m_request.getLocation() + "/");
		m_filename += m_server->getIndexPage(m_locationIndex);
	}

	int fd = WS::open(m_filename, O_RDONLY);
	if (fd == -1)
		return openError(originalFile, isDirectory);

	m_source_fd = fd;
	addFileHeaders();
	m_status = 200;
	m_chunk	 = getResponseAsString();
}

void Response::openError(const std::string& dir, bool isDirectory) {
	bool autoIndex = m_server->isAutoIndex();

	switch (errno) {
		case EACCES:
			throw 403;
		case ENOENT:
		case ENOTDIR:
			if (isDirectory && autoIndex)
				return createIndex(dir);
			throw 404;
		case EMFILE:
			throw 503;
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
			block = read();
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

void Response::createIndex(std::string path_to_index) {
	Entry		root  = { m_request.getLocation(), recursivePathCount(path_to_index) };
	std::string title = "Index of directory: " + root.name;

	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html>\n");
	addToBody("<head><meta charset='utf-8'><title>" + title + "</title></head>\n");
	addToBody("<h1>" + title + "</h1>\n<ul>");

	for (auto& entry : root.subdir)
		addToBody(entry.toString());

	addToBody("</ul></body></html>");

	m_doneReading = true;
	m_status	  = 200;

	m_chunk = getResponseAsString();
}
