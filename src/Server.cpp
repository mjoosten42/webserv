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

	m_locations.push_back(Location());
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
static void
	overwriteIfSpecified(std::string search, size_t& field, size_t defaultVal, t_block_directive *constructor_specs) {
	field						= defaultVal;
	std::string val_from_config = constructor_specs->fetch_simple(search);
	if (!val_from_config.empty())
		field = stringToIntegral<int>(val_from_config);
}

// Short
static void
	overwriteIfSpecified(std::string search, short& field, short defaultVal, t_block_directive *constructor_specs) {
	field						= defaultVal;
	std::string val_from_config = constructor_specs->fetch_simple(search);
	if (!val_from_config.empty())
		field = stringToIntegral<int>(val_from_config);
}

#pragma endregion

Server::Server(t_block_directive *constructor_specs) {
	// INITIALISE MEMBER VARIABLES //
	m_host = "127.0.0.1"; //	The only address we handle requests on is localhost
	// TODO: also parse that optionally from cfg

	//	Nginx default is 80 if super user, otherwise 8000
	overwriteIfSpecified("listen", m_port, 8000, constructor_specs);
	//	Not an Nginx config param
	overwriteIfSpecified("server", m_server_software_name, SERVER_SOFTWARE_DEFAULT_NAME, constructor_specs);
	//	Nginx default: ""
	overwriteIfSpecified("server_name", m_names, "", constructor_specs);
	//	Nginx default: "html" TODO: absolute
	overwriteIfSpecified("root", m_root, "html", constructor_specs);
	if (m_root.back() == '/')
		m_root.pop_back();
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
		if (access(full_path.c_str(), R_OK) != 0)
			LOG_ERR(RED << "WARNING: The custom error pages have been incorrectly configured." << DEFAULT);
		int user_defined_page			= stringToIntegral<int>(*(error_it - 1));
		m_error_page[user_defined_page] = full_path;
		error_it++;
	}

	// Allows user to specify CGI to handle cgi-scripts
	overwriteIfSpecified("cgi", m_CGIs, "", constructor_specs);
	LOG("Server CGI:");
	for (size_t i = 0; i < m_CGIs.size(); i++)
		LOG(m_CGIs[i] + " ");

	// ADD LOCATION BLOCKS, IF PRESENT //
	std::vector<t_block_directive *> location_config_blocks;
	location_config_blocks = constructor_specs->fetch_matching_blocks("location");

	std::vector<t_block_directive *>::iterator it;
	for (it = location_config_blocks.begin(); it != location_config_blocks.end(); ++it)
		m_locations.push_back(Location(*it, this));
}

// For clarity:
//  -We use "Address" to refer to the way the user navigates the server, e.g.
//  webserv.com/images/amogus.jpg
//  -We use "Path" to refer the client-side storage of files, e.g.
//  html/img/amogus.jpg
//  This function tries to find the longest match between the user defined Address
//  and the name of one of the containing location blocks.
size_t Server::getLocationIndexForAddress(const std::string& address) const {
	size_t ret	   = -1;
	size_t longest = 0;

	for (size_t i = 0; i < m_locations.size(); i++) {

		const std::string& loc	   = m_locations[i].m_location;
		size_t			   matched = match(loc, address); // Amount of characters matched

		if (matched == loc.length()) { // I.E. make sure /index doesn't match /images
			if (matched > longest) {
				longest = matched;
				ret		= i;
			}
		}
	}
	return (ret);
}

std::string Server::translateAddressToPath(size_t loc_index, const std::string& file_address) const {
	std::vector<Location>::const_iterator loc = m_locations.begin() + loc_index;
	for (size_t match = 0; match < (loc->m_location.length()); ++match)
		if (loc->m_location[match] != file_address[match])
			return ("");
	return loc->m_root + "/" + file_address.substr(loc->m_location.length());
}

// This function finds the appropriate Location block for a file
// if it is present anywhere on the server.
// It does so without using the Address specified by the user.
size_t Server::getLocationIndexForFile(const std::string& file_to_find) const {
	std::string test_path;
	int			test;

	std::vector<const Location>::iterator l_it = m_locations.begin();
	for (; l_it != m_locations.end(); ++l_it) {
		test_path = l_it->m_root;
		test	  = open((test_path + "/" + file_to_find).c_str(), O_RDONLY);
		if (test != -1) {
			close(test);
			return l_it - m_locations.begin();
		}
	}
	return (-1); // Not present in any Location blocks, default to parent server settings.
}

const std::string& Server::getRoot(size_t loc_index) const {
	if (loc_index == static_cast<size_t>(-1))
		return m_root;
	else
		return m_locations[loc_index].m_root;
}

/*
std::string Server::getRootForFile(size_t loc_index, const std::string& file_to_find) const {
	std::string							  test_path;
	int									  test;
	std::vector<const Location>::iterator l_it = m_locations.begin();

	if (loc_index != static_cast<size_t>(-1)) {
		l_it += loc_index;
		test_path = l_it->m_root;
		test	  = open((test_path + "/" + file_to_find).c_str(), O_RDONLY);
		if (test != -1) {
			close(test);
			return test_path;
		}
	} else {
		for (; l_it != m_locations.end(); ++l_it) {
			test_path = l_it->m_root;
			test	  = open((test_path + "/" + file_to_find).c_str(), O_RDONLY);
			if (test != -1) {
				close(test);
				return test_path;
			}
		}
		test_path = m_root;
		test	  = open((test_path + "/" + file_to_find).c_str(), O_RDONLY);
		if (test != -1) {
			close(test);
			return test_path;
		}
	}
	return ("");
}
*/

bool Server::isCGI(size_t loc, const std::string& ext) const {
	const std::vector<std::string>			*CGIs;
	std::vector<std::string>::const_iterator it;

	if (loc == static_cast<size_t>(-1))
		CGIs = &m_CGIs;
	else
		CGIs = &m_locations[loc].m_CGIs;

	it = std::find(CGIs->begin(), CGIs->end(), ext);
	return it != CGIs->end();
}

#pragma region getters

// returns the host, i.e. the address which the server should listen on.
const std::string& Server::getHost() const {
	return m_host;
}

const std::string& Server::getRoot() const {
	return m_root;
}

const std::string& Server::getServerSoftwareName() const {
	return m_server_software_name;
}

const std::vector<std::string>& Server::getNames() const {
	return m_names;
}

short Server::getPort() const {
	return m_port;
}

size_t Server::getCMB() const {
	return m_client_max_body_size;
}

const std::map<int, std::string>& Server::getErrorPages() const {
	return m_error_page;
}

bool Server::getAutoIndex() const {
	return m_autoindex;
}

#pragma endregion
