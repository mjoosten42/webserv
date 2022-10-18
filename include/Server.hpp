#pragma once

#include "ConfigParser.hpp"
#include "Location.hpp"

#include <string>
#include <vector>

//  https://nginx.org/en/docs/http/ngx_http_core_module.html

class Server {
	public:
		Server();
		Server(std::vector<Server>& servers, t_block_directive *constructor_specs);

		int				   getFD() const;
		short			   getPort() const;
		const std::string& getHost() const;
		const std::string& getName() const;

	private:
		void setupSocket();

	private:
		int					  m_fd; //  Socket_fd
		std::vector<Location> m_locations;

	public:
		std::string m_host; //  TODO: use inet_addr?
		short		m_port;
		std::string m_name;
		std::string m_root;
		std::string m_error_page;
		int			m_client_max_body_size;
};
