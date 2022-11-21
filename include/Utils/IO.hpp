#pragma once

#include "methods.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct pollfd;
class Request;

std::ostream &operator<<(std::ostream &os, const pollfd &pfd);
std::ostream &operator<<(std::ostream &os, const std::pair<int, std::string> &pair);
std::ostream &operator<<(std::ostream &os, const std::pair<methods, bool> &limit_except);
std::ostream &operator<<(std::ostream &os, const std::vector<methods> &methods);
std::ostream &operator<<(std::ostream &os, const Request &request);
