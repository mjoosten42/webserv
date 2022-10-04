#include "Request.hpp"

#include <string>

using std::string;

//  http1.0 mandatory:
//		GET / HTTP/1.0
//
//  http1.1 mandatory [port is optional]
//  	GET / HTTP/1.1
//  		Host: localhost:8080

Request::Request(const string& total) {
	(void)total;
}
