#include "Server.hpp"

#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <fcntl.h> // for serving error files
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> // close

Server::Server() {
	m_host = "localhost";
	m_port = 8080;
	m_names.push_back("webserv.com");
	m_root				   = "html";
	m_client_max_body_size = 0;
	m_autoindex			   = false;

	Location location = Location();
	m_locations.push_back(location);
}

#pragma region overwriteIfSpecified

// String
static void overwriteIfSpecified(std::string		search,
								 std::string	  & field,
								 std::string		defaultVal,
								 t_block_directive *constructor_specs) {
	field						= defaultVal;
	std::string val_from_config = constructor_specs->fetch_simple(search);
	if (!val_from_config.empty())
		field = val_from_config;
}

// String vector
static void overwriteIfSpecified(std::string			   search,
								 std::vector<std::string>& field,
								 std::string			   defaultVal,
								 t_block_directive		  *constructor_specs) {
	std::string content			= defaultVal;
	std::string val_from_config = constructor_specs->fetch_simple(search);
	if (!val_from_config.empty())
		content = val_from_config;
	field = stringSplit(content);
}

// Boolean
static void overwriteIfSpecified(
	std::string search, bool& field, bool defaultVal, std::string nonDefaultVal, t_block_directive *constructor_specs) {
	field						= defaultVal;
	std::string val_from_config = constructor_specs->fetch_simple(search);
	if (!val_from_config.empty() && val_from_config == nonDefaultVal)
		field = !defaultVal;
}

// Integer
static void overwriteIfSpecified(std::string search, int& field, int defaultVal, t_block_directive *constructor_specs) {
	field						= defaultVal;
	std::string val_from_config = constructor_specs->fetch_simple(search);
	if (!val_from_config.empty())
		field = stoi(val_from_config); // TODO replace all stois
}

// Short
static void
	overwriteIfSpecified(std::string search, short& field, short defaultVal, t_block_directive *constructor_specs) {
	field						= defaultVal;
	std::string val_from_config = constructor_specs->fetch_simple(search);
	if (!val_from_config.empty())
		field = stoi(val_from_config); // TODO replace all stois
}

#pragma endregion

Server::Server(t_block_directive *constructor_specs) {
	// INITIALISE MEMBER VARIABLES //
	m_host = "127.0.0.1"; //	The only address we handle requests on is localhost
	// TODO: also parse that optionally from cfg

	//	Nginx default is 80 if super user, otherwise 8000
	overwriteIfSpecified("listen", m_port, 8000, constructor_specs);
	//	Nginx default: ""
	overwriteIfSpecified("server_name", m_names, "", constructor_specs);
	//	Nginx default: "html" TODO: absolute
	overwriteIfSpecified("root", m_root, "./html", constructor_specs);
	//	Nginx default: 0 (which means don't check)
	overwriteIfSpecified("client_max_body_size", m_client_max_body_size, 0, constructor_specs);
	//	Nginx default: false (serve 404 error when navigating ot directory without index.html)
	overwriteIfSpecified("autoindex", m_autoindex, false, "on", constructor_specs);

	// Default error pages are built in, here the user can define their own.
	std::vector<std::string> pages;
	overwriteIfSpecified("error_page", pages, "", constructor_specs);
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

	// ADD LOCATION BLOCKS, IF PRESENT //
	std::vector<t_block_directive *> location_config_blocks;
	location_config_blocks = constructor_specs->fetch_matching_blocks("location");

	std::vector<t_block_directive *>::iterator it;
	for (it = location_config_blocks.begin(); it != location_config_blocks.end(); ++it)
		m_locations.push_back(Location(*it, this));
}

#pragma region getters

// returns the host, i.e. the address which the server should listen on.
const std::string& Server::getHost() const {
	return m_host;
}

const std::string& Server::getRoot() const {
	return m_root;
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
	return m_error_page;
}

const bool& Server::getAutoIndex() const {
	return m_autoindex;
}

#pragma endregion
