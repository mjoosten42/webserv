#pragma once

#include "defines.hpp"

#include <iostream>

#define LOG_ERR(...) std::cerr << std::boolalpha << RED << "[ERROR] " << __VA_ARGS__ << DEFAULT << std::endl

#ifdef DEBUG
#	include <iostream>
#	define LOG(...) std::cout << std::boolalpha << __VA_ARGS__ << std::endl
#else
#	define LOG(x)
#endif
