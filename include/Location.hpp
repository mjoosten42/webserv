#pragma once

#include "ConfigParser.hpp"

#include <iostream>
#include <map>
#include <string>

class Server;

class Location {
	public:
		Location();
		Location(t_block_directive *constructor_specs, Server *parent);

		std::string						   m_location;
		std::map<std::string, std::string> m_cgi_map;
		int								   m_client_max_body_size;
		std::string						   m_limit_except; // IE only allow GET, POST
		std::string						   m_redirect;	   // return 301 $URI
		bool							   m_auto_index;
		std::string						   m_root;
};
