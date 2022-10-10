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

#define STATUS_MESSAGES_LENGTH (sizeof(statusMessages) / sizeof(*statusMessages))

static int compareFunc(int a, int b) {
	return a - b;
}

Response::Response(): HTTP() {}

std::string Response::statusMsg(int code) {
	const char *msg = binarySearchKeyValue<const char *>(code, statusMessages, STATUS_MESSAGES_LENGTH, compareFunc);
	if (msg != nullptr)
		return std::to_string(code) + " " + std::string(msg); //  TODO: c++11
	std::cerr << "Status code not found: " << code << std::endl;
	return "";
}

std::string Response::statusLine() const {
	return "HTTP/1.1" + std::to_string(m_statusCode) + statusMsg(m_statusCode);
}

std::string Response::getResponseAsCPPString(void) const {
	std::map<std::string, std::string>::const_iterator it;
	std::stringstream								   ret;

	ret << statusLine() << CRLF;
	for (it = m_headers.begin(); it != m_headers.end(); it++)
		ret << it->first << " " << it->second << CRLF;
	ret << CRLF;
	ret << m_body << CRLF;

	return (ret.str());
}

void Response::parseStartLine() {}
