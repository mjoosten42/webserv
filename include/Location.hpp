#pragma once

#include "ConfigParser.hpp"

#include <iostream>
#include <string>

class Server;

struct CGI_loc {
		std::string cgi_ext;  // e.g. .py
		std::string cgi_path; // e.g. /usr/bin/python
};

std::ostream& operator<<(std::ostream& o, const CGI_loc& i);

class Location {
	public:
		Location();
		Location(t_block_directive *constructor_specs, Server *parent);

		std::string			 m_location;
		std::vector<CGI_loc> m_cgis_available;
		// std::map<std::string, std::string> cgi_map;
		int			m_client_max_body_size;
		std::string m_limit_except; // IE only allow GET, POST
		std::string m_redirect;		// return 301 $URI
		bool		m_auto_index;
		std::string m_root;
};
