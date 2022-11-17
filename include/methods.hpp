#pragma once

#include <string>

enum methods { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };
const static char *methodStrings[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE" };
const static int methodStringsSize = sizeof(methodStrings) / sizeof(*methodStrings);

std::string toString(methods method);
