#include "MIME.hpp"
#include "Response.hpp"
#include "defines.hpp"
#include "stringutils.hpp"

#include <fcntl.h> // open
#include <sys/socket.h>
#include <unistd.h> // lseek

#define FILE_BUF_SIZE (4096 - 1024) //  the max size of a file we want to load into a buffer in one time.
#define CHUNK_MAX_LENGTH (1024)		//  the desired max length of a HTTP/1.1 Chunked response chunk.

void Response::checkWetherCGI() {
	// TODO
	m_isCGI = strEndsWith(m_request.getLocation(), ".php");
}

bool Response::processNextChunk() {
	if (!m_hasStartedSending) {
		checkWetherCGI();
		handle();
		m_hasStartedSending = true;
	} else {
		getNextChunk();
	}
	return m_isFinalChunk;
}

void Response::handle() {
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
			std::cerr << "Error: method is NONE\n";
	}
}

void Response::sendFail(int code, const std::string& msg) {
	m_statusCode = code;

	initDefaultHeaders();
	addHeader("Content-Type", "text/html");

	addToBody("<h1>" + toString(code) + " " + getStatusMessage() + "</h1>\r\n");
	addToBody("<p>something went wrong somewhere: <b>" + msg + "</b></p>\r\n");

	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}

void Response::sendMoved(const std::string& location) {
	clear(); //  <!-- TODO, also add default server
	initDefaultHeaders();
	m_statusCode			  = 301;
	m_headers["Location"]	  = location;
	m_headers["Connection"]	  = "Close";
	m_headers["Content-Type"] = "text/html";

	addToBody("<h1>" + toString(m_statusCode) + " " + getStatusMessage() + "</h1>\r\n");
	addToBody("<p>The resource has been moved to <a href=\"" + location + "\">" + location + "</a>.</p>");

	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}

void Response::handleGet() {

	//  handleCGI("/usr/bin/perl", "printenv.pl");
	initDefaultHeaders();
	m_statusCode = handleGetWithStaticFile();

	if (m_statusCode != 200)
		sendFail(m_statusCode, "Page is venting");
}

void Response::handlePost() {
	std::string filename = "." + m_request.getLocation();
	ssize_t		bytes_written;

	m_readfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (m_readfd < 0)
		perror("open");
	bytes_written = write(m_readfd, m_request.getBody().c_str(), m_request.getBody().length());
	close(m_readfd);

	m_request.cut(bytes_written);
	m_statusCode = 201;
	addHeader("Location", filename);
	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}

int Response::handleGetWithStaticFile() {

	std::string filename = "." + m_request.getLocation();

	//  TODO: nonblock?
	//  TODO: remove dot
	m_readfd = open(filename.c_str(), O_RDONLY);
	if (m_readfd == -1) {
		//  TODO: correct error codes
		if (errno == EACCES)
			return 403;
		perror("open");
		return 404;
	}

	m_headers["Connection"]	  = "Keep-Alive";
	m_headers["Content-Type"] = MIME::fromFileName(filename);

	return getFirstChunk();
}

const std::string& Response::getChunk() const {
	return m_chunk;
}

//  this function removes bytesSent amount of bytes from the chunk. Used for instance when send() sent
//  less bytes than the chunk's length.
void Response::trimChunk(ssize_t bytesSent) {
	m_chunk = m_chunk.substr(bytesSent, m_chunk.length());
}

bool Response::isInitialized() const {
	return m_hasStartedSending;
}

bool Response::isDone() const {
	return m_isFinalChunk;
}

int Response::getFirstChunk() {

	int ret;

	//  get file size
	//  TODO: don't do this for CGI pipes!
	off_t size = lseek(m_readfd, 0, SEEK_END);
	if (size == -1)
		size = std::numeric_limits<off_t>().max();
	lseek(m_readfd, 0, SEEK_SET); //  set back to start

	if (size > FILE_BUF_SIZE) {
		//  send multichunked
		m_headers["Transfer-Encoding"] = "Chunked";

		m_chunk = getResponseAsString();
		ret		= 200;

	} else {
		//  send in single buf.
		ret			   = addSingleFileToBody();
		m_chunk		   = getResponseAsString();
		m_isFinalChunk = true;
	}
	return ret;
}

int Response::addSingleFileToBody() {
	static char buf[FILE_BUF_SIZE + 1];
	ssize_t		bytes_read;

	bytes_read = read(m_readfd, buf, FILE_BUF_SIZE);
	if (bytes_read == -1) {
		std::cerr << "Reading infile fd " << m_readfd << " failed!\n";
		return 500;
	}
	buf[bytes_read] = 0;
	addToBody(buf, bytes_read);

	close(m_readfd);

	return 200;
}

void Response::getNextChunk() {
	static char buf[CHUNK_MAX_LENGTH];
	ssize_t		size;

	m_chunk.clear();
	size = read(m_readfd, buf, CHUNK_MAX_LENGTH);

	if (size == -1) {
		std::cerr << "Reading infile failed!\n";

		//  TODO
		m_isFinalChunk = true;
		close(m_readfd);
		return;
	}

	//  if we have reached EOF, then we finish the multichunked response with empty data.
	if (size == 0) {
		m_chunk		   = "0\r\n\r\n";
		m_isFinalChunk = true;
		close(m_readfd);
		return;
	}

	//  add the size of the chunk, and finish the buffer with CRLF
	{
		std::stringstream ss;

		ss.seekp(std::ios::beg);
		ss << std::hex << size;

		m_chunk = ss.str() + CRLF;
		m_chunk.append(buf, size);
		m_chunk += CRLF;
	}
}
