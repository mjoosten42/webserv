#pragma once

#include "ConfigParser.hpp"
#include "methods.hpp"

#include <iostream>
#include <map>
#include <string>

class Server;

class Location {
		friend class Server;

	public:
		Location();

		void add(t_block_directive *constructor_specs);

		std::string getLocationAsString(std::string tabs) const;

	private:
		std::string m_location;

		std::string				   m_root;		   // root html
		std::string				   m_indexPage;	   // index index.html
		std::map<int, std::string> m_error_pages;  // error_page 404 404.html
		std::vector<methods>	   m_limit_except; // limit_except GET POST
		std::string				   m_redirect;	   // redirect /cgi-bin/
		std::vector<std::string>   m_CGIs;		   // cgi php pl
		std::string				   m_uploadDir;	   // upload /uploads
		size_t					   m_client_max_body_size;
		bool					   m_auto_index;
};
