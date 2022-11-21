#pragma once

#include <string>

enum methods { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, INVALID };

const static char *methodStrings[] = {
	"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "INVALID"
};
const static int methodStringsSize = sizeof(methodStrings) / sizeof(*methodStrings);

std::string toString(methods method);
methods		toMethod(const std::string	  &str);
