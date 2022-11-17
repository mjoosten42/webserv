#pragma once

#include "defines.hpp"

#include <cerrno>	// errno
#include <string.h> // strerror

#ifdef DEBUG
#	include <iostream>

#	define LOG(x) std::cout << std::boolalpha << x << std::endl
#	define LOG_ERR(x) std::cerr << std::boolalpha << RED << "[ERROR] " << x << DEFAULT << std::endl
#else
#	define LOG(x)
#	define LOG_ERR(x)
#endif
