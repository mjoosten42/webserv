#include "Request.hpp"

#include <fstream>
#include <iostream> // TODO:remove
#include <sstream>
#include <string>
#include <sys/socket.h> // send()

//  http1.1 mandatory [port is optional]
//  	GET / HTTP/1.1
//  		Host: localhost:8080

Request::Request(int fd, const Server *server, const std::string& total): m_fd(fd), m_server(server) {
	std::stringstream ss(total);
	std::string		  method;

	ss >> method;
	ss >> m_location;
	m_method = NONE;
	if (method == std::string("GET"))
		m_method = GET;
	m_headers["Host"] = "localhost:8080";
	(void)m_fd;
	(void)m_server;
}
