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

		std::string m_location;

		std::string m_root;
		std::string m_indexPage;
		std::string m_limit_except; // IE only allow GET, POST

		std::map<int, std::string> m_redirects; // redirect /cgi-bin/
		std::vector<std::string>   m_CGIs;

		int	 m_client_max_body_size;
		bool m_auto_index;
};
