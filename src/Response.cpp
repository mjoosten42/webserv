#include "Response.hpp"

#include "utils.hpp"

#include <sstream>
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

std::string Response::statusMessage(int code) const {
	const char *msg = binarySearchKeyValue<const char *>(code, statusMessages, statusMessagesSize);
	if (msg != nullptr)
		return msg;
	std::cerr << "Status code not found: " << code << std::endl;
	exit(EXIT_FAILURE);
}

std::string Response::statusLine() const {
	return "HTTP/1.1" + std::to_string(m_statusCode) + statusMessage(m_statusCode);
}

std::string Response::getResponseAsCPPString(void) const {
	std::map<std::string, std::string>::const_iterator it;
	std::stringstream								   ret;

	ret << statusLine() << CRLF;
	for (it = m_headers.begin(); it != m_headers.end(); it++)
		ret << it->first << ": " << it->second << CRLF;
	ret << CRLF;
	ret << m_body << CRLF;

	return (ret.str());
}

void Response::parseStartLine() {}

void Response::setStatusCode(int code) {
	m_statusCode = code;
}
