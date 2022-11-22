#pragma once

#include "ConfigParser.hpp"
#include "methods.hpp"

#include <map>
#include <string>
#include <vector>

template <typename T, typename F>
void overwriteIfSpecified(const std::string &search, T &field, t_block_directive *constructor_specs, F fun) {
	std::string value = constructor_specs->fetch_simple(search);

	if (!value.empty())
		field = fun(value);
}

bool					   toBool(const std::string &str);
std::string				   copy(const std::string &str);
std::vector<methods>	   toMethods(const std::string &str);
std::map<unsigned int, std::string> toMap(const std::string &str);