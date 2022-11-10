#pragma once

#include "defines.hpp"

#ifdef DEBUG
#	include <iostream>

#	define LOG(x) std::cout << std::boolalpha << x << std::endl
#	define LOG_ERR(x) std::cerr << std::boolalpha << "[ERROR] " << x << std::endl
#else
#	define LOG(x)
#	define LOG_ERR(x)
#endif
