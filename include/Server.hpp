#pragma once

#include "ConfigParser.hpp"
#include "Location.hpp"

#include <array>
#include <map>
#include <string>
#include <vector>

// https://nginx.org/en/docs/http/ngx_http_core_module.html

class Server {
	public:
		Server();
		Server(t_block_directive *constructor_specs);

		size_t			  getLocationIndexForAddress(const std::string			 &address_to_find) const;
		const std::string translateAddressToPath(size_t loc_index, std::string file_address) const;
		size_t			  getLocationIndexForFile(const std::string file_to_find) const;
		const std::string getRootForFile(const size_t loc_index, const std::string file_to_find) const;

		const std::string			  & getRoot() const;
		const std::string			  & getHost() const;
		const std::string			  & getServerSoftwareName() const;
		const std::vector<std::string>& getNames() const;

		const int						& getCMB() const;
		short							  getPort() const;
		const std::map<int, std::string>& getErrorPages() const;
		const bool						& getAutoIndex() const;

		bool checkWhetherCGI(const std::string& requested_file) const; //TODO: Move?

	private:
		std::vector<Location>			   m_locations;
		std::map<std::string, std::string> m_cgi_map;
		std::string						   m_host;	// the IP address this server listens on. TODO: use inet_addr?
		short							   m_port;	// port the server listens on
		std::vector<std::string>		   m_names; // i.e. example.com www.example.com etc.
		std::string						   m_root;
		std::string						   m_server_software_name; // i.e. amogus
		std::map<int, std::string>		   m_error_page;
		int								   m_client_max_body_size;
		bool							   m_autoindex;
};
