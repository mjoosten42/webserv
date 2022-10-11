#include "Response.hpp"

#include "stringutils.hpp"
#include "utils.hpp"

#include <sys/socket.h>

struct Status {
		int			key;
		const char *value;
};

const static Status statusMessages[] = {
	{ 200, "OK" }, { 400, "Bad Request" }, { 403, "Forbidden" }, { 404, "Not Found" }, { 500, "Internal Server Error" },
};

const static int statusMessagesSize = sizeof(statusMessages) / sizeof(*statusMessages);

Response::Response(): HTTP() {}

void Response::reset() {
	HTTP::reset();
	m_statusCode = 0;
}

int Response::getStatusCode() const {
	return m_statusCode;
}

std::string Response::getStatusMessage() const {
	const char *msg = binarySearchKeyValue(m_statusCode, statusMessages, statusMessagesSize);
	if (msg != nullptr)
		return msg;
	std::cerr << "Status code not found: " << m_statusCode << std::endl;
	exit(EXIT_FAILURE);
}

std::string Response::getStatusLine() const {
	return "HTTP/1.1 " + toString(m_statusCode) + " " + getStatusMessage();
}

std::string Response::getResponseAsCPPString(void) const {
	std::map<std::string, std::string>::const_iterator it;
	std::stringstream								   ret;

	ret << getStatusLine() << CRLF;
	for (it = m_headers.begin(); it != m_headers.end(); it++)
		ret << it->first << ": " << it->second << CRLF;
	ret << CRLF;
	ret << m_body;

	return (ret.str());
}

void Response::parseStartLine() {}

void Response::setStatusCode(int code) {
	m_statusCode = code;
}

void Response::addToBody(const std::string& str) {
	m_body += str;
}

void Response::addHeader(const std::string& field, const std::string& value) {
	m_headers[field] = value;
}
