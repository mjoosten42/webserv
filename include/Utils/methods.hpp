#pragma once

#include <string>

enum methods { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH, INVALID };

const static char *methodStrings[] = { "GET",	  "HEAD",	 "POST",  "PUT",   "DELETE",
									   "CONNECT", "OPTIONS", "TRACE", "PATCH", "INVALID" };

std::string toString(methods method);
methods		toMethod(const std::string &str);
