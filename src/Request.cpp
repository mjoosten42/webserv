#include "Request.hpp"

#include <string>

//  http1.1 mandatory [port is optional]
//  	GET / HTTP/1.1
//  		Host: localhost:8080

Request::Request(int fd, const std::string& total) {
	(void)total;
	m_fd			  = fd;
	m_method		  = GET;
	m_headers["Host"] = "localhost:8080";
	m_body			  = "amogus";
}
