#include "Location.hpp"

#include "Server.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

void Location::getLimitExceptFromConfig(t_block_directive *constructor_specs) {
	std::string				 val_from_config;
	std::vector<std::string> methods_as_strings = { "GET", "POST", "DELETE" };

	val_from_config = constructor_specs->fetch_simple("limit_except");
	if (!val_from_config.empty())
		methods_as_strings = stringSplit(val_from_config);

	// if (std::find(methods_as_strings.begin(), methods_as_strings.end(), "GET") != methods_as_strings.end())
	// 	m_limit_except.push_back(GET);
	// if (std::find(methods_as_strings.begin(), methods_as_strings.end(), "POST") != methods_as_strings.end())
	// 	m_limit_except.push_back(POST);
	// if (std::find(methods_as_strings.begin(), methods_as_strings.end(), "DELETE") != methods_as_strings.end())
	// 	m_limit_except.push_back(DELETE);

	for (auto method : methods_as_strings) { // included the non-auto method above should we need to revert back to it.
		if (method == "GET")
			m_limit_except.push_back(GET);
		if (method == "POST")
			m_limit_except.push_back(POST);
		if (method == "DELETE")
			m_limit_except.push_back(DELETE);
	}
	return;
}

Location::Location(): m_location("/"), m_root("html"), m_indexPage("index.html"), m_client_max_body_size(0) {}

Location::Location(t_block_directive *constructor_specs, Server *parent):
	m_location("/"),
	m_root(parent->getRoot()),
	m_indexPage(parent->getIndexPage()),
	m_client_max_body_size(parent->getCMB()) {

	std::string val_from_config;

	val_from_config = constructor_specs->additional_params;
	if (!val_from_config.empty())
		m_location = val_from_config;

	if (m_location.back() != '/') // TODO: check if correct
		m_location.push_back('/');

	val_from_config = constructor_specs->fetch_simple("client_max_body_size");
	if (!val_from_config.empty())
		m_client_max_body_size = stringToIntegral<int>(val_from_config);

	getLimitExceptFromConfig(constructor_specs);
	for (auto except : m_limit_except)
		LOG(except);

	val_from_config = constructor_specs->fetch_simple("autoindex");
	if (!val_from_config.empty())
		m_auto_index = (val_from_config == "on" ? true : false);

	val_from_config = constructor_specs->fetch_simple("index");
	if (!val_from_config.empty())
		m_indexPage = val_from_config;

	m_redirect = constructor_specs->fetch_simple("redirect");

	val_from_config = constructor_specs->fetch_simple("root");
	if (!val_from_config.empty())
		m_root = val_from_config;
	if (m_root.back() == '/')
		m_root.pop_back();

	// Allows user to specify CGI to handle cgi-scripts
	m_CGIs = stringSplit(constructor_specs->fetch_simple("cgi"));
}
