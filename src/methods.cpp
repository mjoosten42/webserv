#include "methods.hpp"
#include <string>
#include "logger.hpp"

std::string	toString(methods method) {
	size_t index = static_cast<size_t>(method);
	if (index >= methodStringsSize) {
		LOG_ERR("Method not found: " << method);
		return "";
	}
	return methodStrings[index];
}
