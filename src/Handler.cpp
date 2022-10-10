#include "Handler.hpp"

#include <iostream> // TODO: remove
#include <string>
#include <sys/socket.h>
#include <unistd.h> // close

Handler::Handler(): m_fd(make_shared(-1)), m_server(NULL) {}

Handler::Handler(int fd, const Server *server): m_fd(make_shared(fd)), m_server(server) {
	(void)m_server;
}

void Handler::sendResponse() const {
	std::string response = m_response.getResponseAsCPPString();
	if (send(m_fd, response.c_str(), response.length(), 0) == -1)
		perror("send");
}
