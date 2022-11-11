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
		LOG("error: " << error);
		m_statusCode = error;
		serveError(getStatusMessage());
	}
}

void Response::handleDelete() {
	std::string absolute = WS::realpath(m_filename);

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

	if (isDirectory) {
		if (m_filename.back() != '/') // CPP11
			return sendMoved(m_request.getLocation() + "/");
		m_filename += m_server->getIndexPage(m_locationIndex);
		LOG("index at: " + m_filename);
	}

	m_source_fd = WS::open(m_filename, O_RDONLY);
	if (m_source_fd == -1)
		return openError(originalFile, isDirectory);

	addFileHeaders();
	m_statusCode = 200;
	m_chunk		 = getResponseAsString();
}

void Response::openError(const std::string& dir, bool isDirectory) {
	bool autoIndex = m_server->getAutoIndex();

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
	ssize_t		bytes_read = WS::read(m_source_fd, BUFFER_SIZE - m_chunk.size());
	std::string block;

	LOG(RED "Read: " DEFAULT << bytes_read);
	switch (bytes_read) {
		case -1:
		case 0:
			m_doneReading = true;
			WS::close(m_source_fd);
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
	Entry entry = { m_request.getLocation(), recursivePathCount(path_to_index) };

	addHeader("Content-Type", "text/html");

	addToBody("<h1> Index of directory: " + entry.name + "</h1>\n<ul>");

	for (size_t i = 0; i < entry.subdir.size(); i++)
		addToBody(entry.subdir[i].toString());

	m_doneReading = true;
	m_statusCode  = 200;

	m_chunk = getResponseAsString();
}
