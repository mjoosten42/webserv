#include "methods.hpp"

#include "logger.hpp"

#include <string>

std::string toString(methods method) {
	size_t index = static_cast<size_t>(method);
	if (index >= methodStringsSize) {
		LOG_ERR("Method not found: " << method);
		return "";
	}
	return methodStrings[index];
}

methods toMethod(const std::string& str) {
	for (int i = 0; i < methodStringsSize; i++)
		if (str == methodStrings[i])
			return static_cast<methods>(i);
	return static_cast<methods>(-1);
}