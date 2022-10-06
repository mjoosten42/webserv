#include "Request.hpp"

#include <fstream>
#include <iostream> // TODO:remove
#include <sstream>
#include <string>
#include <sys/socket.h> // send()

//  http1.1 mandatory [port is optional]
//  	GET / HTTP/1.1
//  		Host: localhost:8080

Request::Request(int fd, const Server *server): m_fd(fd), m_server(server), m_progress(NONE) {
	m_headers["Host"] = "localhost:8080";
	(void)m_fd;
	(void)m_server;
}

void Request::addToRequest(const std::string& str) {
	std::stringstream ss(str);
	std::string		  tmp;

	switch (m_progress) {
		case NONE:
			ss >> tmp;
			if (tmp == "GET")
				m_method = GET;
			break;

		default:
			break;
	}
}
