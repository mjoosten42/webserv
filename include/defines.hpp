#pragma once

//  HTTP RFC newline
#define CRLF "\r\n"

//  Terminal colors
#define RED "\033[0;31m"
#define DEFAULT "\033[0m"

#define HTTP_VERSION "HTTP/1.1"
#define CGI_VERSION "CGI/1.1"

#include <iostream>

#if DEBUG
#	define LOG(x) std::cout << std::boolalpha << x << std::endl
#	define LOG_ERR(x) std::cerr << __FILE__ << ':' << __LINE__ << " " x << std::endl
#else
#	define LOG(x)
#	define LOG_ERR(x)
#endif
