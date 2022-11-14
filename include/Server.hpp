#pragma once

#include "ConfigParser.hpp"
#include "Location.hpp"
#include "cpp109.hpp"

#include <map>
#include <string>
#include <vector>

// https://nginx.org/en/docs/http/ngx_http_core_module.html

class Server {
	public:
		Server();
		Server(t_block_directive *constructor_specs);

		int getLocationIndex(const std::string& address_to_find) const;

		std::string translateAddressToPath(int loc_index, const std::string& file_address) const;

		const std::string& getRoot(int loc_index = -1) const;
		const std::string& getHost() const;
		const std::string& getIndexPage(int loc_index = -1) const;
		const std::string& getRedirect(const std::string& address) const;

		const std::vector<std::string>	& getNames() const;
		const std::map<int, std::string>& getErrorPages() const;

		size_t getCMB() const;
		short  getPort() const;
		bool   getAutoIndex() const;

		bool isCGI(int loc, const std::string& ext) const;

	private:
		std::vector<Location>	   m_locations;
		std::vector<std::string>   m_CGIs;
		std::string				   m_host;	// the IP address this server listens on. TODO: use inet_addr?
		short					   m_port;	// port the server listens on
		std::vector<std::string>   m_names; // i.e. example.com www.example.com etc.
		std::string				   m_root;
		std::map<int, std::string> m_error_page;
		size_t					   m_client_max_body_size;
		bool					   m_autoindex;
		std::string				   m_indexPage;
};
