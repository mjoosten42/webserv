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

	overwriteIfSpecified("root", m_root, constructor_specs, copy);
	overwriteIfSpecified("index", m_indexPage, constructor_specs, copy);
	overwriteIfSpecified("client_max_body_size", m_client_max_body_size, constructor_specs, stringToIntegral<size_t>);
	overwriteIfSpecified("limit_except", m_limit_except, constructor_specs, toMethods);
	overwriteIfSpecified("redirect", m_redirect, constructor_specs, copy);
	overwriteIfSpecified("cgi", m_CGIs, constructor_specs, stringSplit);
	overwriteIfSpecified("error_page", m_error_pages, constructor_specs, toMap);
	overwriteIfSpecified("autoindex", m_auto_index, constructor_specs, toBool);
	overwriteIfSpecified("upload", m_uploadDir, constructor_specs, copy);
}
