#include "Location.hpp"
#include "Server.hpp"

Location::Location() {
	m_location			   = "/";
	m_client_max_body_size = -1;
	m_limit_except		   = "";
	m_redirect			   = "";
	m_auto_index		   = false;
	m_root				   = "html";
}

Location::Location(t_block_directive *constructor_specs, Server *parent) {
	std::string val_from_config;

	m_location		= "";
	val_from_config = constructor_specs->additional_params;
	if (!val_from_config.empty())
		m_location = val_from_config;

	m_client_max_body_size = parent->m_client_max_body_size;
	val_from_config		   = constructor_specs->fetch_simple("client_max_body_size");
	if (!val_from_config.empty())
		m_client_max_body_size = stoi(val_from_config);

	m_limit_except	= "";
	val_from_config = constructor_specs->fetch_simple("limit_except");
	if (!val_from_config.empty())
		m_limit_except = val_from_config;

	m_redirect = ""; //  Not a standard Nginx config param

	m_auto_index	= false;
	val_from_config = constructor_specs->fetch_simple("autoindex");
	if (!val_from_config.empty())
		m_auto_index = (val_from_config.compare("on") == 0 ? true : false);

	m_root			= parent->m_root;
	val_from_config = constructor_specs->fetch_simple("root");
	if (!val_from_config.empty())
		m_root = val_from_config;
}