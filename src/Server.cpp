#include "Server.hpp"

#include "defines.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <fcntl.h> // for serving error files
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> // close

Server::Server() {
	m_host = "localhost";
	m_port = 8080;
	m_name = "webserv.com"; //  Nginx default = ""
	m_root = "html";
	// m_error_page[404] = "./html/404.html";

	Location location = Location();
	m_locations.push_back(location);
}

Server::Server(t_block_directive *constructor_specs) {
	//  INITIALISE MEMBER VARIABLES //
	m_host = "127.0.0.1"; //	The only address we handle requests on is localhost
	//  TODO: also parse that optionally from cfg

	std::string val_from_config;
	m_port			= 8080; //	Nginx default is 80 if super user, otherwise 8000
	val_from_config = constructor_specs->fetch_simple("listen");
	if (!val_from_config.empty())
		m_port = stoi(val_from_config); // TODO replace all stois

	std::string name = ""; //  Nginx default: ""
	val_from_config	 = constructor_specs->fetch_simple("server_name");
	if (!val_from_config.empty())
		name = val_from_config;

	m_names = stringSplit(name);

	m_name			= "Amogus"; // TODO: Delete?
	val_from_config = constructor_specs->fetch_simple("server");
	if (!val_from_config.empty())
		m_name = val_from_config;

	m_root			= "./html"; //	Nginx default: "html" TODO: absolute
	val_from_config = constructor_specs->fetch_simple("root");
	if (!val_from_config.empty())
		m_root = val_from_config;

	// m_error_page[404] = "./html/404.html"; //	Our default 404 error page

	val_from_config								= constructor_specs->fetch_simple("error_page");
	std::vector<std::string>		   pages	= stringSplit(val_from_config);
	std::vector<std::string>::iterator error_it = pages.begin();
	while (error_it != pages.end()) {
		error_it++;
		if (error_it == pages.end())
			break;
		std::string full_path = m_root + "/" + *error_it;
		// print("Checking user-defined error page: " + full_path);
		int user_defined_page = open((full_path).c_str(), O_RDONLY);
		if (user_defined_page == -1)
			LOG_ERR(RED << "WARNING: The custom error pages have been incorrectly configured." << DEFAULT);
		else
			close(user_defined_page);
		user_defined_page				= stoi(*(error_it - 1)); // TODO:: check this for non numeric values
		m_error_page[user_defined_page] = full_path;
		error_it++;
	}

	//  Default value for Nginx, if set to 0 means don't check.
	//  "Setting size to 0 disables checking of client request body size."
	m_client_max_body_size = 0;
	val_from_config		   = constructor_specs->fetch_simple("client_max_body_size");
	if (!val_from_config.empty())
		m_client_max_body_size = stoi(val_from_config);

	//  ADD LOCATION BLOCKS, IF PRESENT //
	std::vector<t_block_directive *> location_config_blocks;
	location_config_blocks = constructor_specs->fetch_matching_blocks("location");

	std::vector<t_block_directive *>::iterator it;
	for (it = location_config_blocks.begin(); it != location_config_blocks.end(); ++it)
		m_locations.push_back(Location(*it, this));
}

#pragma region getters

//  returns the host, i.e. the address which the server should listen on.
const std::string& Server::getHost() const {
	return m_host;
}

const std::string& Server::getRoot() const {
	return m_root;
}

//  TODO: fix confusing name
const std::string& Server::getName() const {
	return m_name;
}

const std::vector<std::string>& Server::getNames() const {
	return m_names;
}

short Server::getPort() const {
	return m_port;
}

const int& Server::getCMB() const {
	return m_client_max_body_size;
}

const std::map<int, std::string>& Server::getErrorPages() const {
	return (m_error_page);
}

#pragma endregion
