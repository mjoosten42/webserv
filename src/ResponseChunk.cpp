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
	switch (m_request.getMethod()) {
		case GET:
			handleGet();
			break;
		case POST:
			handlePost();
			break;
		case DELETE:
			break;
		default:
			LOG_ERR("Invalid method");
	}
}

void Response::setFlags() {
	if (m_request.hasHeader("Connection"))
		m_close = (m_request.getHeaderValue("Connection") == "close");
	m_isCGI = (MIME::getExtension(m_request.getLocation()) == "php");
}

void Response::handleGet() {

	addDefaultHeaders();
	if (m_isCGI) {
		// TODO: parse from config
		m_statusCode = m_cgi.start(m_request, m_server, "/usr/bin/perl", "printenv.pl");

		// wait(nullptr);

		m_headers["Transfer-Encoding"] = "Chunked";
		m_readfd					   = m_cgi.popen.readfd;
		m_isCGIProcessingHeaders	   = true;

		m_chunk = getStatusLine() + getHeadersAsString();

	} else {
		m_statusCode = handleGetWithStaticFile();
	}

	if (m_statusCode != 200)
		serveError(m_statusCode);
	// sendFail(m_statusCode, m_isCGI ? "CGI BROKE 😂😂😂" : "Page is venting");
}

void Response::handlePost() {
	std::string filename = m_server->getRoot() + m_request.getLocation();
	ssize_t		bytes_written;

	m_readfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (m_readfd < 0)
		perror("open");
	bytes_written = write(m_readfd, m_request.getBody().data(), m_request.getBody().length());
	close(m_readfd);

	m_request.cut(bytes_written);
	m_statusCode = 201;
	addHeader("Location", m_request.getLocation());
	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}

// TODO: Fix this. I added an ugly hacky param in case the file served is supposed ot be an error page.
int Response::handleGetWithStaticFile(std::string file) {
	std::string filename = m_server->getRoot() + m_request.getLocation();
	if (!file.empty())
		filename = file;
	// filename = m_server->getRoot() + "/" + file;
	LOG("Handle static: " + filename);

	m_readfd = open(filename.c_str(), O_RDONLY);
	if (m_readfd == -1) {
		if (errno == EACCES)
			return 403;
		return 404;
	}

	addHeader("Content-Type", MIME::fromFileName(filename));

	return getFirstChunk();
}

//  this function removes bytesSent amount of bytes from the chunk. Used for instance when send() sent
//  less bytes than the chunk's length.
void Response::trimChunk(ssize_t bytesSent) {
	m_chunk.erase(0, bytesSent);
}

bool Response::isDone() const {
	return m_isFinalChunk;
}

// gets the first chunk of a static file
int Response::getFirstChunk() {
	off_t size = lseek(m_readfd, 0, SEEK_END);
	int	  ret;

	//  get file size
	//  TODO: don't do this for CGI pipes!
	if (size == -1)
		size = std::numeric_limits<off_t>().max();
	lseek(m_readfd, 0, SEEK_SET); //  set back to start

	if (size > BUFFER_SIZE) { //  send multichunked
		addHeader("Transfer-Encoding", "Chunked");

		m_chunk = getResponseAsString();
		ret		= 200;

	} else { //  send in single buf.
		ret			   = addSingleFileToBody();
		m_chunk		   = getResponseAsString();
		m_isFinalChunk = true;
	}
	return ret;
}

int Response::addSingleFileToBody() {
	ssize_t bytes_read = read(m_readfd, buf, BUFFER_SIZE);

	switch (bytes_read) {
		case -1:
			perror("read");
			return 500;
		default:
			addToBody(buf, bytes_read);
	}
	close(m_readfd);
	return 200;
}

void Response::getCGIHeaderChunk() {

	std::string block = readBlockFromFile();

	size_t pos = m_headerEndFinder.find(block);
	if (pos == std::string::npos) {
		m_chunk += block;
	} else {
		m_isCGIProcessingHeaders = false;
		std::string headers		 = block.substr(0, pos);
		block					 = block.substr(pos);
		m_chunk += headers + wrapStringInChunkedEncoding(block);
	}
}

std::string& Response::getNextChunk() {

	if (m_isFinalChunk || m_chunk.size() > BUFFER_SIZE)
		return m_chunk;

	if (m_isCGIProcessingHeaders) {
		getCGIHeaderChunk();
		return m_chunk;
	}

	std::string block = readBlockFromFile();
	m_chunk += wrapStringInChunkedEncoding(block);
	return m_chunk;
}

std::string Response::wrapStringInChunkedEncoding(std::string& str) {
	return toHex(str.length()) + CRLF + str + CRLF;
}

std::string Response::readBlockFromFile() {
	std::string block;
	ssize_t		bytes_read = read(m_readfd, buf, BUFFER_SIZE - m_chunk.size());
	switch (bytes_read) {
		case -1:
			perror("read");
		case 0:
			m_isFinalChunk = true;
			close(m_readfd);
			break;
		default:
			block.append(buf, bytes_read);
	}
	return block;
}
