#include "HTTP.hpp"

#include "utils.hpp"

#include <iostream> // TODO:remove
#include <sstream>
#include <string>

//  Note: according to RFC, \r\n is the correct newline
//  However, it recommends parsers to also consider just \n sufficient
//  HTTP 1.1 mandatory [port is optional]
//  	GET / HTTP/1.1
//  		Host: localhost:8080

HTTP::HTTP(int fd, const Server *server): m_fd(fd), m_server(server) {
	(void)m_server;
	(void)m_fd;
}
