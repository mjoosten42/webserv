#include "Request.hpp"

#include "utils.hpp"

#include <fstream>
#include <iostream> // TODO:remove
#include <sstream>
#include <string>
#include <sys/socket.h> // send()

//  http1.1 mandatory [port is optional]
//  	GET / HTTP/1.1
//  		Host: localhost:8080

Request::Request(int fd, const Server *server): m_fd(fd), m_server(server), m_progress(METHOD), m_body("") {
	m_headers["Host"] = "localhost:8080";
	(void)m_server;
	(void)m_fd;
}

void Request::addToRequest(const std::string& str) {
	std::istringstream is;
	std::string		   tmp;

	is.str(str);
	is >> tmp;
	if (tmp == "GET")
		m_method = GET;
	else
		std::cerr << "Unkown request method: " << tmp << std::endl;
	is >> m_location;
	is >> tmp;
	if (tmp != "HTTP/1.1")
		std::cout << "Invalid HTTP: " << tmp << std::endl;

	is >> tmp;
	is >> m_headers[tmp];
	std::cout << "Tmp: " << tmp << std::endl;
	while (is.eof()) {
		is >> tmp;
		m_body += tmp.c_str();
	}

	const char *names[] = { "GET", "POST", "DELETE" };
	std::cout << "[METHOD] " << names[m_method] << std::endl;
	std::cout << "[LOCATION] " << m_location << std::endl;
	std::cout << "[HEADERS] ";
	printMap(m_headers);
	std::cout << "[BODY] " << m_body << std::endl;
	(void)m_progress;
}
