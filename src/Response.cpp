#include "Response.hpp"

#include "Server.hpp"
#include "defines.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

struct Status {
		int			key;
		const char *value;
};

const static Status statusMessages[] = { { 200, "OK" },
										 { 201, "Created" },
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

const static int statusMessagesSize = sizeof(statusMessages) / sizeof(*statusMessages);

//  TO DETERMINE WHICH SERVER:
//  in connection, a map of servers and hostnames. get the hostname from the request(as it is required),
//  and just pass the correct server into the response. That's it.
//  the hard part is the map and the socket creation(no duplicate sockets for same host/port)
//  perhaps just loop over the survurs and check hostname/port?

Response::Response():
	HTTP(),
	m_statusCode(200),
	m_server(NULL),
	m_readfd(-1),
	m_isFinalChunk(false),
	m_isCGI(false),
	m_isCGIProcessingHeaders(false),
	m_hasReadFDPoller(false) {}

void Response::clear() {
	HTTP::clear();
	m_statusCode = 0;
	m_headerEndFinder.reset();
}

void Response::addServer(const Server *server) {
	m_server = server;
}

std::string Response::getStatusMessage() const {
	const char *msg = binarySearchKeyValue(m_statusCode, statusMessages, statusMessagesSize);
	if (msg != NULL)
		return msg;
	LOG_ERR("Status code not found: " << m_statusCode);
	exit(EXIT_FAILURE);
}

std::string Response::getStatusLine() const {
	return std::string(HTTP_VERSION) + " " + toString(m_statusCode) + " " + getStatusMessage() + CRLF;
}

//  Returns the response as a string to send over a socket. When there is a body present,
//  the body is amended automatically and Content-Length is calculated.
std::string Response::getResponseAsString() {
	std::string response;

	if (!hasHeader("Transfer-Encoding"))
		addHeader("Content-Length", toString(m_body.length()));

	response += getStatusLine();
	response += getHeadersAsString();
	response += CRLF;
	response += getBody();

	return (response);
}

void Response::addDefaultHeaders() {
	addHeader("Hostname", m_request.getHost());
	if (!m_server->getName().empty())
		addHeader("Server", m_server->getName());
}

Request& Response::getRequest() {
	return m_request;
}

const Server *Response::getServer() const {
	return m_server;
}

bool Response::shouldClose() const {
	return m_close;
}

bool Response::readfdNeedsPoll() const {
	return m_hasReadFDPoller;
}

int Response::getReadFD() const {
	return m_readfd;
}
