#include "Response.hpp"

#include "MIME.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "file.hpp" // extension
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

struct Status {
		unsigned int key;
		const char	*value;
};

const static Status statusMessages[] = { { 200, "OK" },
										 { 201, "Created" },
										 { 204, "No Content" },
										 { 301, "Moved Permanently" },
										 { 400, "Bad Request" },
										 { 403, "Forbidden" },
										 { 404, "Not Found" },
										 { 405, "Method Not Allowed" },
										 { 413, "Payload Too Large" },
										 { 500, "Internal Server Error" },
										 { 501, "Not Implemented" },
										 { 502, "Bad Gateway" },
										 { 503, "Service Unavailable" },
										 { 505, "HTTP Version Not Supported" } };

const static int statusMessagesSize = sizeof(statusMessages) / sizeof(*statusMessages);

Response::Response():
	HTTP(),
	m_server(NULL),
	m_processedRequest(false),
	m_isCGI(false),
	m_hadFD(false),
	m_isChunked(false),
	m_doneReading(false),
	m_headersDone(false),
	m_close(false) {}

void Response::addServer(const Server *server) {
	m_server = server;
}

void Response::initialize() {
	m_locationIndex = m_server->getLocationIndex(m_request.getLocation());
	m_filename		= m_server->getRoot(m_locationIndex) + m_request.getLocation();

	if (!m_request.getLocation().empty())
		if (m_request.getLocation().back() == '/')
			m_filename += m_server->getIndexPage(m_locationIndex);

	setFlags();
	addDefaultHeaders();
}

void Response::setFlags() {
	std::string ext = extension(m_filename);

	m_isCGI = (m_server->isCGI(m_locationIndex, ext));
	m_close = (m_request.getHeaderValue("Connection") == "close");
}

void Response::addDefaultHeaders() {
	addHeader("Server", SERVER_SOFTWARE);
	if (m_request.hasHeader("Host"))
		addHeader("Host", m_request.getHost());
}

std::string Response::getStatusMessage() const {
	const char *msg = binarySearchKeyValue(m_status, statusMessages, statusMessagesSize);
	if (msg != NULL)
		return msg;
	LOG_ERR("Status code not found: " << m_status);
	return "Status code not found " + toString(m_status);
}

std::string Response::getStatusLine() const {
	return HTTP_VERSION " " + toString(m_status) + " " + getStatusMessage() + CRLF;
}

// Returns the response as a string to send over a socket. When there is a body present,
// the body is amended automatically and Content-Length is calculated.
std::string Response::getResponseAsString() {
	std::string response;

	if (!hasHeader("Transfer-Encoding") && !hasHeader("Content-Length"))
		addHeader("Content-Length", toString(m_body.length()));

	response += getStatusLine();
	response += getHeadersAsString();
	response += CRLF;
	response += getBody();

	return response;
}

Request &Response::getRequest() {
	return m_request;
}

FD Response::getSourceFD() const {
	return m_source_fd;
}

bool Response::wantsClose() const {
	return m_close;
}

bool Response::hasProcessedRequest() const {
	return m_processedRequest;
}

bool Response::isCGI() const {
	return m_isCGI;
}

bool Response::hadFD() const {
	return m_hadFD;
}
