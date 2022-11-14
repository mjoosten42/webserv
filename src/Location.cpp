#include "Location.hpp"

#include "Server.hpp"
#include "cpp109.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

Location::Location():
	m_location("/"),
	m_CGIs(stringSplit("")),
	m_client_max_body_size(-1),
	m_limit_except(""), //not yet implemented
	m_root("html"),
	m_auto_index(false),
	m_indexPage("index.html"),
	m_is_redirected(false),
	m_redirection_path("") {}

Location::Location(t_block_directive *constructor_specs, Server *parent):
	m_location("/"),
	m_client_max_body_size(parent->getCMB()),
	m_root(parent->getRoot()),
	m_indexPage(parent->getIndexPage()),
	m_is_redirected(false),
	m_redirection_path("") {

	std::string val_from_config;

	val_from_config = constructor_specs->additional_params;
	if (!val_from_config.empty())
		m_location = val_from_config;

	if (my_back(m_location) != '/') // TODO: check if correct
		m_location.push_back('/');

	val_from_config = constructor_specs->fetch_simple("client_max_body_size");
	if (!val_from_config.empty())
		m_client_max_body_size = stringToIntegral<int>(val_from_config);

	val_from_config = constructor_specs->fetch_simple("limit_except");
	if (!val_from_config.empty())
		m_limit_except = val_from_config;

	val_from_config = constructor_specs->fetch_simple("autoindex");
	if (!val_from_config.empty())
		m_auto_index = (val_from_config == "on" ? true : false);

	val_from_config = constructor_specs->fetch_simple("index");
	if (!val_from_config.empty())
		m_indexPage = val_from_config;

	val_from_config = constructor_specs->fetch_simple("redirect");
	if (!val_from_config.empty())
	{
		m_is_redirected = true;
		m_redirection_path = val_from_config;
	}

	val_from_config = constructor_specs->fetch_simple("root");
	if (!val_from_config.empty())
		m_root = val_from_config;
	if (my_back(m_root) == '/')
		my_pop_back(m_root);

	// Allows user to specify CGI to handle cgi-scripts
	m_CGIs = stringSplit(constructor_specs->fetch_simple("cgi"));

	LOG("Location CGI: ");
	for (size_t i = 0; i < m_CGIs.size(); i++)
		LOG("\t" + m_CGIs[i] + " ");
}
