#include "Server.hpp"

#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

Server::Server() {
	init();

	m_locations.push_back(Location());
}

void Server::init() {
	m_host				   = "127.0.0.1";
	m_port				   = 8080;
	m_root				   = "html";
	m_client_max_body_size = 0;
	m_autoindex			   = false;
	m_indexPage			   = "index.html";

	m_names.push_back("webserv.com");
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
	init();

	overwriteIfSpecified("listen", m_port, 8080, constructor_specs);
	overwriteIfSpecified("server_name", m_names, "", constructor_specs);
	overwriteIfSpecified("root", m_root, "html", constructor_specs);
	overwriteIfSpecified("client_max_body_size", m_client_max_body_size, 0, constructor_specs);
	overwriteIfSpecified("autoindex", m_autoindex, false, "on", constructor_specs);
	overwriteIfSpecified("index", m_indexPage, "index.html", constructor_specs);

	// Default error pages are built in, here the user can define their own.
	std::vector<std::string> pages;
	overwriteIfSpecified("error_page", pages, "", constructor_specs);
	std::vector<std::string>::iterator error_it = pages.begin();
	while (error_it != pages.end()) {
		error_it++;
		if (error_it == pages.end())
			break;
		std::string full_path			= m_root + "/" + *error_it;
		int			user_defined_page	= stringToIntegral<int>(*(error_it - 1));
		m_error_page[user_defined_page] = full_path;
		error_it++;
	}

	overwriteIfSpecified("cgi", m_CGIs, "", constructor_specs);

	for (auto& location : constructor_specs->fetch_matching_blocks("location"))
		m_locations.push_back(Location(location, this));
}

// For clarity:
//  -We use "Address" to refer to the way the user navigates the server, e.g.
//  webserv.com/images/amogus.jpg
//  -We use "Path" to refer the client-side storage of files, e.g.
//  html/images/amogus.jpg
//  This function tries to find the longest match between the user defined Address
//  and the name of one of the containing location blocks.
int Server::getLocationIndex(const std::string& address) const {
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

std::string Server::translateAddressToPath(int loc_index, const std::string& file_address) const {
	if (loc_index == -1)
		return m_root + file_address;
	else
		return m_locations[loc_index].m_root + file_address;
}

bool Server::isCGI(int loc_index, const std::string& ext) const {
	const std::vector<std::string> *CGIs;

	if (loc_index == -1)
		CGIs = &m_CGIs;
	else
		CGIs = &m_locations[loc_index].m_CGIs;

	return std::find(CGIs->begin(), CGIs->end(), ext) != CGIs->end();
}

#pragma region getters

// returns the host, i.e. the address which the server should listen on.
const std::string& Server::getHost() const {
	return m_host;
}

const std::string& Server::getRoot(int loc_index) const {
	if (loc_index == -1)
		return m_root;
	else
		return m_locations[loc_index].m_root;
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

const std::string& Server::getErrorPage(int code) const {
	return m_error_page.at(code);
}

bool Server::isAutoIndex() const {
	return m_autoindex;
}

const std::string& Server::getIndexPage(int loc_index) const {
	if (loc_index == -1)
		return m_indexPage;
	else
		return m_locations[loc_index].m_indexPage;
}

const std::string& Server::getRedirect(int loc_index) const {
	return m_locations[loc_index].m_redirect;
}

bool Server::hasErrorPage(int code) const {
	return m_error_page.find(code) != m_error_page.end();
}

bool Server::isRedirect(int loc_index) const {
	return loc_index != -1 && !m_locations[loc_index].m_redirect.empty();
}

bool Server::allowsMethod(int loc_index, methods method) const {
	if (loc_index == -1)
		return true; // Can't limit_except a server block.
	else {
		auto allowed = m_locations[loc_index].m_limit_except;
		return (std::find(allowed.begin(), allowed.end(), method) != allowed.end());
	}
}

#pragma endregion
