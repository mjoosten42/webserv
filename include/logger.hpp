#pragma once

#include "defines.hpp"

#ifdef DEBUG
#	include <iostream>

#	define LOG(x) std::cout << std::boolalpha << x << std::endl
#	define LOG_ERR(x) std::cerr << __FILE__ << ':' << __LINE__ << " " << x << std::endl
#else
#	define LOG(x)
#	define LOG_ERR(x)
#endif

template <typename T>  
void	logVector(const std::vector<T> & vector) {
	typename std::vector<T>::const_iterator it = vector.begin();
	for (; it != vector.end(); ++it){
		LOG(*it);
	}
}
