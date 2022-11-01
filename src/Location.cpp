#include "Location.hpp"

#include "Server.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

Location::Location(): m_location("/"), m_client_max_body_size(-1), m_root("html") {}

Location::Location(t_block_directive *constructor_specs, Server *parent):
	m_location("/"), m_client_max_body_size(parent->getCMB()), m_root(parent->getRoot()) {

	std::string val_from_config;

	val_from_config = constructor_specs->additional_params;
	if (!val_from_config.empty())
		m_location = val_from_config;
	if (m_location.back() != '/')
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

	val_from_config = constructor_specs->fetch_simple("root");
	if (!val_from_config.empty())
		m_root = val_from_config;
	if (m_root.back() == '/')
		m_root.pop_back();

	// Allows user to specify CGI to handle cgi-scripts
	std::vector<std::string>		   cgi_specs = stringSplit(constructor_specs->fetch_simple("cgi"));
	std::vector<std::string>::iterator cgi_it	 = cgi_specs.begin();
	while (cgi_it != cgi_specs.end() && cgi_it + 1 != cgi_specs.end()) {
		m_cgi_map[*cgi_it] = *(cgi_it + 1);
		cgi_it += 2;
	}
	LOG("Location CGI:");
	for (std::map<std::string, std::string>::iterator m_it = m_cgi_map.begin(); m_it != m_cgi_map.end(); ++m_it)
		LOG(m_it->first + " " + m_it->second);
}
