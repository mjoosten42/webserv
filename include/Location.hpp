#pragma once

#include "ConfigParser.hpp"
#include "Request.hpp"

#include <iostream>
#include <map>
#include <string>

class Server;

class Location {
	public:
		Location();
		Location(t_block_directive *constructor_specs, Server *parent);

		std::string m_location;

		std::string			 m_root; // root html
		std::string			 m_indexPage; // index index.html
		std::vector<methods> m_limit_except; // limit_except GET POST
		std::string			 m_redirect;	 // redirect /cgi-bin/
		std::vector<std::string> m_CGIs; // cgi php pl

		int	 m_client_max_body_size;
		bool m_auto_index;

	private:
		void getLimitExceptFromConfig(t_block_directive *constructor_specs);
};
