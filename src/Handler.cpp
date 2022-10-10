#include "Handler.hpp"

#include <iostream> // TODO: remove
#include <string>
#include <sys/socket.h>
#include <unistd.h> // close

Handler::Handler(): m_fd(-1), m_server(NULL) {}

Handler::Handler(int fd, const Server *server): m_fd(fd), m_server(server) {
	(void)m_server;
}

Handler::~Handler() {
	//  if (close(m_fd < 0)) {
	//  	std::cerr << "Fd " << m_fd << ": ";
	//  	perror("close");
	//  }
}

void Handler::sendResponse() const {
	std::string response = m_response.getResponseAsCPPString();
	if (send(m_fd, response.c_str(), response.length(), 0) == -1)
		perror("send");
}
