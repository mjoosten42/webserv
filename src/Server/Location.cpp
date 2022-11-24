#include "Location.hpp"

#include "methods.hpp"
#include "overwrite.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

Location::Location():
	m_location("/"),
	m_root("html"),
	m_indexPage("index.html"),
	m_limit_except({ GET, POST, DELETE }),
	m_uploadDir("/uploads"),
	m_client_max_body_size(0),
	m_auto_index(false) {}

void Location::add(t_block_directive *constructor_specs) {
	if (!constructor_specs->additional_params.empty())
		m_location = constructor_specs->additional_params;

	overwriteIfSpecified("cgi", m_CGIs, constructor_specs, stringSplit);
	overwriteIfSpecified("root", m_root, constructor_specs, copy);
	overwriteIfSpecified("index", m_indexPage, constructor_specs, copy);
	overwriteIfSpecified("upload", m_uploadDir, constructor_specs, copy);
	overwriteIfSpecified("redirect", m_redirect, constructor_specs, copy);
	overwriteIfSpecified("autoindex", m_auto_index, constructor_specs, toBool);
	overwriteIfSpecified("error_page", m_error_pages, constructor_specs, toMap);
	overwriteIfSpecified("limit_except", m_limit_except, constructor_specs, toMethods);
	overwriteIfSpecified("client_max_body_size", m_client_max_body_size, constructor_specs, stringToIntegral<size_t>);
}

const std::string &Location::getLocation() const {
	return m_location;
}

// Sort by location length, but make sure '/' ends up at the front
bool operator<(const Location &lhs, const Location &rhs) {
	if (lhs.m_location.front() == '/' && rhs.m_location.front() != '/')
		return true;
	return lhs.m_location.size() < rhs.m_location.size();
}
