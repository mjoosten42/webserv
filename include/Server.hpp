#pragma once

#include "shared_fd.hpp"

#include <string>
#include <vector>

class Location {
	public:
		std::string m_location;

		int			m_client_max_body_size;
		std::string m_limit_except; //  IE only allow GET, POST
		std::string m_redirect;		//  return 301 $URI
		bool		m_auto_index;
		std::string m_root;
};

//  https://nginx.org/en/docs/http/ngx_http_core_module.html

class Server {
	public:
		Server();
		Server(int port);

		int getFD() const;

	private:
		shared_fd			  m_fd;
		std::vector<Location> m_locations;

		//  Config

	private:
		std::string m_host; //  TODO: use inet_addr?
		int			m_port;
		std::string m_name;
		std::string m_root;
		std::string m_error_page;
		//  int m_client_max_body_size;
};
