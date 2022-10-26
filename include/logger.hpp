#pragma once

#include "defines.hpp"

#include <iostream>

#ifdef DEBUG
#	define LOG(x) std::cout << std::boolalpha << x << std::endl
#	define LOG_ERR(x) std::cerr << __FILE__ << ':' << __LINE__ << " " x << std::endl
#else
#	define LOG(x)
#	define LOG_ERR(x)
#endif
