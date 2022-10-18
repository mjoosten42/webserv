#pragma once

#include "ConfigParser.hpp"

#include <string>
#include <vector>

class Server;

class Location {
	public:
		Location();
		Location(t_block_directive *constructor_specs, Server *parent);

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

		//  Config

	private:
		std::string m_host; //  TODO: use inet_addr?
		short		m_port;
		std::string m_name;
		std::string m_root;
		std::string m_error_page;
		//  int m_client_max_body_size;
};
