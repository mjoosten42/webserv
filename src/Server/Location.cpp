#include "Location.hpp"

#include "defines.hpp"
#include "methods.hpp"
#include "overwrite.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

const static char *location_directives[] = {
	"root",		"client_max_body_size", "autoindex", "index", "error_page", "upload", "cgi",
	"redirect", "limit_except"
	// "server_name", "listen",
};

Location::Location():
	m_location("/"),
	m_root("html"),
	m_indexPage("index.html"),
	m_limit_except({ GET, POST, DELETE }),
	m_uploadDir("/uploads"),
	m_client_max_body_size(0),
	m_auto_index(false) {}

void Location::add(block_directive *constructor_specs) {
	if (!constructor_specs->additional_params.empty())
		m_location = constructor_specs->additional_params;
	// if (m_location.front() != '/')
	// 	throw (std::invalid_argument("Location blocks must start with a leading '/'."));

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

bool Location::isAllowedLocationDirective(const std::string &str) const {
	for (size_t i = 0; i < SIZEOF_ARRAY(location_directives); i++)
		if (str == location_directives[i])
			return true;
	return false;
}

bool Location::hasOnlyAllowedDirectives(block_directive *constructor_specs) const {
	auto it = constructor_specs->simple_directives.begin();
	for (; it != constructor_specs->simple_directives.end(); ++it)
		if (!isAllowedLocationDirective(it->name))
			throw(std::invalid_argument("Directive \"" + it->name + "\" is not allowed in this context."));
	return true;
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
