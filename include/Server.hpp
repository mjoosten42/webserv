#pragma once

#include "ConfigParser.hpp"
#include "Location.hpp"

#include <string>
#include <vector>

//  https://nginx.org/en/docs/http/ngx_http_core_module.html

//  TODO: multiple hosts per server!
class Server {
	public:
		Server();
		Server(t_block_directive *constructor_specs);

		const std::string& getName() const;
		const std::string& getRoot() const;
		const std::string& getHost() const;

		const int& getCMB() const;
		short	   getPort() const;

	private:
		std::vector<Location> m_locations;

	private:
		std::string m_host; //  TODO: use inet_addr?
		short		m_port;
		std::string m_name;
		std::string m_root;
		std::string m_error_page;
		int			m_client_max_body_size;
};
