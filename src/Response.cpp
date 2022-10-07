#include "Response.hpp"

#include "utils.hpp"

#include <sstream>
#include <sys/socket.h>

struct Status {
		int			code;
		const char *message;
};

const static Status statusMessages[] = { { 200, "OK" }, { 404, "Not Found" } };

Response::Response(): HTTP(-1, NULL) {}

Response::Response(int fd, const Server *server): HTTP(fd, server) {}

std::string Response::statusMsg(int code) const {
	const static int size = sizeof(statusMessages) / sizeof(*statusMessages);

	for (int i = 0; i < size; i++)
		if (statusMessages[i].code == code)
			return statusMessages[i].message;
	std::cerr << "Status code not found: " << code << std::endl;
	return "";
}

std::string Response::statusLine() const {
	return "HTTP/1.1" + std::to_string(m_statusCode) + statusMsg(m_statusCode);
}

bool Response::sendResponse(void) const {
	std::string response = getResponseAsCPPString();
	if (send(m_fd, response.c_str(), response.length(), 0) == -1)
		fatal_perror("send");
	return (true);
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
