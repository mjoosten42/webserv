#pragma once

// HTTP RFC newline
#define CRLF "\r\n"

#define IFS " \n\r\t\f\v"

// Terminal colors
#define RED "\033[0;31m"
#define DEFAULT "\033[0m"

#define HTTP_VERSION "HTTP/1.1"
#define CGI_VERSION "CGI/1.1"
#define SERVER_SOFTWARE "WebSus/1.1"

#include <map>
#include <string>

typedef std::map<std::string, std::string>::const_iterator MapIter;
