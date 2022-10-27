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
	m_isCGI = (MIME::getExtension(m_request.getLocation()) == "php");
}

void Response::handleGet() {
	LOG(RED "Handle Get" DEFAULT);

	if (m_isCGI)
		handleGetCGI();
 	else
		m_statusCode = handleGetWithFile();

	if (m_statusCode != 200)
		serveError(m_statusCode);
	// sendFail(m_statusCode, m_isCGI ? "CGI BROKE 😂😂😂" : "Page is venting")
}

// TODO: send to CGI
void Response::handlePost() {
	std::string filename = m_server->getRoot() + m_request.getLocation();

	LOG(RED "Handle Post: " DEFAULT + filename);

	addHeader("Location", m_request.getLocation());
	m_statusCode = 418;
	m_doneReading = true;
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
	m_doneReading = true;
}

// TODO: Fix this. I added an ugly hacky param in case the file served is supposed ot be an error page.
int Response::handleGetWithFile(std::string file) {
	std::string filename = m_server->getRoot() + m_request.getLocation();
	if (!file.empty())
		filename = file;
	// filename = m_server->getRoot() + "/" + file;
	LOG("Get: File: " + filename);

	m_readfd = open(filename.c_str(), O_RDONLY);
	if (m_readfd == -1) {
		if (errno == EACCES)
			return 403;
		return 404;
	}

	addHeader("Content-Type", MIME::fromFileName(filename));

	return getFirstChunk();
}

void	Response::handleGetCGI() {
	LOG(RED "Handle CGI" DEFAULT);
	// TODO: parse from config
	m_statusCode = m_cgi.start(m_request, m_server, "/usr/bin/perl", "printenv.pl");

	addHeader("Transfer-Encoding", "Chunked");
	m_readfd				 = m_cgi.popen.readfd;
	m_CGI_DoneProcessingHeaders = false;

	close(m_cgi.popen.writefd); // close for now, we are not doing anything with it

	m_chunk = getStatusLine() + getHeadersAsString();
}

// this function removes bytesSent amount of bytes from the beginning of the chunk.
void Response::trimChunk(ssize_t bytesSent) {
	m_chunk.erase(0, bytesSent);
}

bool Response::isDone() const {
	return m_doneReading && m_chunk.empty();
}

// gets the first chunk of a static file
int Response::getFirstChunk() {
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

	return 200;
}

void Response::getCGIHeaderChunk() {

	std::string block = readBlockFromFile();

	size_t pos = m_headerEndFinder.find(block);
	if (pos == std::string::npos) {
		m_chunk += block;
	} else {
		m_CGI_DoneProcessingHeaders = true;
		std::string headers		 = block.substr(0, pos);
		block					 = block.substr(pos);
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

// EXAMPLE USAGE ONLY:
void Response::autoIndex() {
	m_statusCode = 200;

	addHeader("Content-Type", "text/html");
	addToBody(autoIndexHtml(m_server->getRoot()));

	m_doneReading = true;
}
