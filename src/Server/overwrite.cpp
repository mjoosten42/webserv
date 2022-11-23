#include "overwrite.hpp"

#include "methods.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

bool toBool(const std::string &str) {
	return str == "on";
}

std::string copy(const std::string &str) {
	return str;
}

std::vector<methods> toMethods(const std::string &str) {
	std::vector<methods> vec;

	for (auto &str : stringSplit(str)) {
		methods method = toMethod(str);
		if (method != INVALID)
			vec.push_back(method);
	}
	return vec;
}

std::map<unsigned int, std::string> toMap(const std::string &str) {
	std::map<unsigned int, std::string> map;
	std::vector<std::string>			vec = stringSplit(str);

	for (size_t i = 0; i + 1 < vec.size(); i++) {
		unsigned int error = stringToIntegral<unsigned int>(vec[i++]);
		map[error]		   = vec[i];
	}
	return map;
}
