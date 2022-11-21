#include "methods.hpp"

#include "logger.hpp"

#include <string>

std::string toString(methods method) {
	return methodStrings[static_cast<size_t>(method)];
}

methods toMethod(const std::string &str) {
	for (int i = 0; i < methodStringsSize; i++)
		if (str == methodStrings[i])
			return static_cast<methods>(i);
	return INVALID;
}