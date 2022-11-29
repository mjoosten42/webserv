#pragma once

#include "ConfigParser.hpp"
#include "Location.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>

// https://nginx.org/en/docs/http/ngx_http_core_module.html

class Server {
	public:
		Server();

		void add(block_directive *constructor_specs);

		int getLocationIndex(const std::string &address_to_find) const;

		// Server
		const std::vector<std::string> &getNames() const;
		const std::string			   &getHost() const;
		unsigned short					getPort() const;

		// Location
		const std::string &getRoot(int loc_index) const;
		const std::string &getIndexPage(int loc_index) const;
		const std::string &getRedirect(int loc_index) const;
		const std::string &getErrorPage(int loc_index, int code) const;
		const std::string &getUploadDir(int loc_index) const;
		size_t			   getCMB(int loc_index) const;

		bool allowsMethod(int loc_index, methods method) const;
		bool hasErrorPage(int loc_index, int code) const;
		bool isAutoIndex(int loc_index) const;
		bool isRedirect(int loc_index) const;
		bool isCGI(int loc_index, const std::string &ext) const;

		std::string getServerAsString(std::string tabs = "") const;
		std::string getAllowedMethodsAsString(int loc_index) const;

		bool isAllowedContextDirective(const std::string &str, const char **list) const;
		bool hasOnlyAllowedDirectives(block_directive *constructor_specs, const char **list) const;

	private:
		std::string				 m_host;  // the IP address this server listens on.
		short					 m_port;  // port the server listens on
		std::vector<std::string> m_names; // i.e. example.com www.example.com etc.
		std::vector<Location>	 m_locations;
};
