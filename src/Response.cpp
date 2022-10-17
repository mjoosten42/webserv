#include "Response.hpp"

#include "MIME.hpp"
#include "defines.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <fcntl.h> // open
#include <sys/socket.h>
#include <unistd.h> // lseek

struct Status {
		int			key;
		const char *value;
};

const static Status statusMessages[] = { { 200, "OK" },
										 { 301, "Moved Permanently" },
										 { 400, "Bad Request" },
										 { 403, "Forbidden" },
										 { 404, "Not Found" },
										 { 413, "Payload Too Large" },
										 { 418, "I'm a teapot" },
										 { 500, "Internal Server Error" },
										 { 501, "Not Implemented" },
										 { 502, "Bad Gateway" },
										 { 505, "HTTP Version Not Supported" } };

const static int statusMessagesSize	 = sizeof(statusMessages) / sizeof(*statusMessages);

Response::Response():
	HTTP(),
	m_statusCode(-1),
	m_request(),
	m_server(NULL),
	m_readfd(-1),
	m_hasStartedSending(false),
	m_isFinalChunk(false) {}

Response::Response(Request request, const Server *server):
	HTTP(),
	m_statusCode(-1),
	m_request(request),
	m_server(server),
	m_readfd(-1),
	m_hasStartedSending(false),
	m_isFinalChunk(false) {}

void Response::clear() {
	HTTP::clear();
	m_statusCode = 0;
}

std::string Response::getStatusMessage() const {
	const char *msg = binarySearchKeyValue(m_statusCode, statusMessages, statusMessagesSize);
	if (msg != NULL)
		return msg;
	std::cerr << "Status code not found: " << m_statusCode << std::endl;
	exit(EXIT_FAILURE);
}

std::string Response::getStatusLine() const {
	return "HTTP/1.1 " + toString(m_statusCode) + " " + getStatusMessage() + CRLF;
}

const std::string& Response::getChunk() const {
	return m_chunk;
}

bool Response::isInitialized() const {
	return m_hasStartedSending;
}

bool Response::isDone() const {
	return m_isFinalChunk;
}

void Response::handle() {
	switch (m_request.getMethod()) {
		case GET:
			handleGet();
			break;
		case POST:
			break;
		case DELETE:
			break;
		default:
			std::cerr << "Error: method is NONE\n";
	}
}

void Response::initDefaultHeaders() {

	//  TODO
	//  m_headers["Hostname"] = *m_request.getHeaders().find("Hostname");
	//  m_headers["Server"] = m_server-> TDO
	m_headers["Server"] = "AMOGUS";
	(void)m_server;
}

void Response::handleGet() {

	//  handleCGI("/usr/bin/perl", "printenv.pl");
	initDefaultHeaders();
	m_statusCode = 200;
	m_statusCode = handleGetWithStaticFile();

	if (m_statusCode != 200)
		sendFail(m_statusCode, "Page is venting");
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

int Response::handleGetWithStaticFile() {

	std::string filename = "." + m_request.getLocation();

	//  TODO: nonblock?
	//  TODO: remove dot
	m_readfd			 = open(filename.c_str(), O_RDONLY);
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

#define FILE_BUF_SIZE (4096 - 1024)

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

		m_chunk						   = getResponseAsString();
		ret							   = 200;

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
	ssize_t		bytesRead;

	bytesRead = read(m_readfd, buf, FILE_BUF_SIZE);
	if (bytesRead == -1) {
		std::cerr << "Reading infile fd " << m_readfd << " failed!\n";
		return 500;
	}
	buf[bytesRead] = 0;
	addToBody(buf);

	close(m_readfd);

	return 200;
}

bool Response::processNextChunk() {
	if (!m_hasStartedSending) {
		handle();
		m_hasStartedSending = true;
	} else {
		getNextChunk();
	}
	return m_isFinalChunk;
}

//  Returns the response as a string to send over a socket. When there is a body present,
//  the body is amended automatically and Content-Length is calculated.
std::string Response::getResponseAsString() {
	std::string response;

	response += getStatusLine();
	if (m_body.length() > 0)
		addHeader("Content-Length", toString(m_body.length()));
	response += getHeadersAsString();
	response += CRLF;
	response += getBody();

	return (response);
}

std::string Response::getHeadersAsString() const {
	std::map<std::string, std::string>::const_iterator it;
	std::string										   headers;

	for (it = m_headers.begin(); it != m_headers.end(); it++)
		headers += it->first + ": " + it->second + CRLF;
	return headers;
}

#define CHUNK_MAX_LENGTH 1024

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
