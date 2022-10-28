#include "Location.hpp"

#include "Server.hpp"
#include "logger.hpp"
#include "stringutils.hpp"

std::ostream& operator<<(std::ostream& o, const CGI_loc& i) {
	o << "Extension: " << i.cgi_ext << std::endl;
	o << "Path: " << i.cgi_path << std::endl;
	return (o);
}

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

	m_client_max_body_size = parent->getCMB();
	val_from_config		   = constructor_specs->fetch_simple("client_max_body_size");
	if (!val_from_config.empty())
		m_client_max_body_size = stoi(val_from_config);

	m_limit_except	= "";
	val_from_config = constructor_specs->fetch_simple("limit_except");
	if (!val_from_config.empty())
		m_limit_except = val_from_config;

	m_redirect = ""; // Not a standard Nginx config param

	m_auto_index	= false;
	val_from_config = constructor_specs->fetch_simple("autoindex");
	if (!val_from_config.empty())
		m_auto_index = (val_from_config.compare("on") == 0 ? true : false);

	m_root			= parent->getRoot();
	val_from_config = constructor_specs->fetch_simple("root");
	if (!val_from_config.empty())
		m_root = val_from_config;

	// Allows user to specify CGI to handle cgi-scripts
	std::vector<std::string>		   cgi_specs = stringSplit(constructor_specs->fetch_simple("cgi"));
	std::vector<std::string>::iterator cgi_it	 = cgi_specs.begin();
	while (cgi_it != cgi_specs.end()) {
		cgi_it++;
		if (cgi_it == cgi_specs.end())
			break;
		CGI_loc tmp;
		tmp.cgi_path = *cgi_it;
		tmp.cgi_ext = *(cgi_it - 1);
		m_cgis_available.push_back(tmp);
		cgi_it++;
	}
	LOG("Location CGI:");
	logVector(m_cgis_available);
}
