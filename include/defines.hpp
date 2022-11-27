#pragma once

// HTTP RFC newline
#define CRLF "\r\n"

// Whitespace
#define IFS " \n\r\t\f\v"

// Size of array literal
#define SIZEOF_ARRAY(x) (sizeof(x) / sizeof(*x))

// Terminal colors
#define DEFAULT "\033[0m"
#define BLACK "\033[0;30m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define WHITE "\033[0;37m"

// Versions
#define CGI_VERSION "CGI/1.1"
#define HTTP_VERSION "HTTP/1.1"
#define SERVER_SOFTWARE "WebSus/1.1"
