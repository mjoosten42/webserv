#include "Location.hpp"

#include "Server.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

Location::Location():
	m_location("/"),
	m_root("html"),
	m_indexPage("index.html"),
	m_limit_except({ GET, POST, DELETE }),
	m_client_max_body_size(0),
	m_auto_index(false) {}

std::string copy(const std::string& str) {
	return str;
}

std::vector<std::string> toVec(const std::string& str) {
	return stringSplit(str);
}

std::vector<methods> toMethods(const std::string& str) {
	std::vector<methods> vec;

	for (auto& str : stringSplit(str)) {
		methods method = toMethod(str);
		if (method != static_cast<methods>(-1))
			vec.push_back(method);
	}
	return vec;
}

std::map<int, std::string> toMap(const std::string& str) {
	std::map<int, std::string> map;
	std::vector<std::string>   vec = stringSplit(str);

	for (size_t i = 0; i + 1 < vec.size(); i++) {
		int error  = stringToIntegral<int>(vec[i++]);
		map[error] = vec[i];
	}
	return map;
}

bool toBool(const std::string& str) {
	return str == "on";
}

void Location::add(t_block_directive *constructor_specs) {
	m_location = constructor_specs->additional_params;

	overwriteIfSpecified("root", m_root, constructor_specs, copy);
	overwriteIfSpecified("index", m_indexPage, constructor_specs, copy);
	overwriteIfSpecified("client_max_body_size", m_client_max_body_size, constructor_specs, stringToIntegral<size_t>);
	overwriteIfSpecified("limit_except", m_limit_except, constructor_specs, toMethods);
	overwriteIfSpecified("redirect", m_redirect, constructor_specs, copy);
	overwriteIfSpecified("cgi", m_CGIs, constructor_specs, toVec);
	overwriteIfSpecified("error_page", m_error_pages, constructor_specs, toMap);
	overwriteIfSpecified("autoindex", m_auto_index, constructor_specs, toBool);
}

template <typename T, typename F>
void Location::overwriteIfSpecified(const std::string& search, T& field, t_block_directive *constructor_specs, F fun) {
	std::string value = constructor_specs->fetch_simple(search);

	if (!value.empty())
		field = fun(value);
}
