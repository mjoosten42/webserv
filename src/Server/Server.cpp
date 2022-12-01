#include "Server.hpp"

#include "IO.hpp"
#include "Location.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "overwrite.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <algorithm> // sort

const static char *server_directives[] = {
	"server_name", "listen",	 "root",  "client_max_body_size", "autoindex",
	"index",	   "error_page", "upload"
	// "cgi", "redirect", "limit_except"
};

const static char *location_directives[] = {
	"root",		"client_max_body_size", "autoindex", "index", "error_page", "upload", "cgi",
	"redirect", "limit_except"
	// "server_name", "listen",
};

Server::Server(): m_host("127.0.0.1"), m_port(8000), m_names({ { "webserv.com" } }), m_locations(1) {}

void Server::add(const block_directive &constructor_specs) {
	hasOnlyAllowedDirectives(constructor_specs, server_directives, SIZEOF(server_directives)); // will throw if not

	m_locations.front().add(constructor_specs);

	overwriteIfSpecified("listen", m_port, constructor_specs, stringToIntegral<short>);
	overwriteIfSpecified("server_name", m_names, constructor_specs, stringSplit);

	for (auto &block : constructor_specs.fetch_matching_blocks("location")) {
		if (block.name != "location")
			continue;
		m_locations.push_back(m_locations.front()); // Copy server default to new location
		if (block.params.empty())
			throw(std::invalid_argument("All location blocks must have a name."));
		hasOnlyAllowedDirectives(block, location_directives, SIZEOF(location_directives));
		m_locations.back().add(block); // Add config
	}

	// Overwrite default '/' with configs '/' if provided
	std::sort(m_locations.begin(), m_locations.end());
	if (m_locations.size() >= 2)
		if (m_locations[1].getLocation() == "/")
			m_locations.erase(m_locations.begin());
}

// For clarity:
//  -We use "Address" to refer to the way the user navigates the server, e.g.
//  webserv.com/images/amogus.jpg
//  -We use "Path" to refer the client-side storage of files, e.g.
//  html/images/amogus.jpg
//  This function tries to find the longest match between the user defined Address
//  and the name of one of the containing location blocks.
int Server::getLocationIndex(const std::string &address) const {
	size_t longest = 0;
	int	   ret	   = 0;

	for (size_t i = 0; i < m_locations.size(); i++) {
		const std::string &loc	   = m_locations[i].m_location;
		size_t			   matched = match(loc, address); // Amount of characters matched

		if (matched == loc.length()) { // Make sure /index.html doesn't match /images
			if (matched > longest) {
				longest = matched;
				ret		= i;
			}
		}
	}
	return (ret);
}

bool Server::isCGI(int loc_index, const std::string &ext) const {
	auto CGIs = m_locations[loc_index].m_CGIs;

	return std::find(CGIs.begin(), CGIs.end(), ext) != CGIs.end();
}

bool Server::isAllowedContextDirective(const std::string &str, const char *list[], size_t size) const {
	for (size_t i = 0; i < size; i++)
		if (str == list[i])
			return true;
	return false;
}

bool Server::hasOnlyAllowedDirectives(const block_directive &constructor_specs, const char *list[], size_t size) const {
	for (auto &simple : constructor_specs.simple_directives)
		if (!isAllowedContextDirective(simple.name, list, size))
			throw(std::invalid_argument("Directive \"" + simple.name + "\" is not allowed in this context."));
	return true;
}

#pragma region getters

// Server

const std::vector<std::string> &Server::getNames() const {
	return m_names;
}

// returns the host, i.e. the address which the server should listen on.
const std::string &Server::getHost() const {
	return m_host;
}

unsigned short Server::getPort() const {
	return m_port;
}

// Location

const std::string &Server::getRoot(int loc_index) const {
	return m_locations[loc_index].m_root;
}

const std::string &Server::getIndexPage(int loc_index) const {
	return m_locations[loc_index].m_indexPage;
}

const std::string &Server::getRedirect(int loc_index) const {
	return m_locations[loc_index].m_redirect;
}

const std::string &Server::getErrorPage(int loc_index, int code) const {
	return m_locations[loc_index].m_error_pages.at(code);
}

const std::string &Server::getUploadDir(int loc_index) const {
	return m_locations[loc_index].m_uploadDir;
}

size_t Server::getCMB(int loc_index) const {
	return m_locations[loc_index].m_client_max_body_size;
}

bool Server::isAutoIndex(int loc_index) const {
	return m_locations[loc_index].m_auto_index;
}

bool Server::hasErrorPage(int loc_index, int code) const {
	auto &pages = m_locations[loc_index].m_error_pages;

	return pages.find(code) != pages.end();
}

bool Server::isRedirect(int loc_index) const {
	return !m_locations[loc_index].m_redirect.empty();
}

bool Server::allowsMethod(int loc_index, methods method) const {
	auto &methods = m_locations[loc_index].m_limit_except;

	return std::find(methods.begin(), methods.end(), method) != methods.end();
}

std::string Server::getAllowedMethodsAsString(int loc_index) const {
	auto &le = m_locations[loc_index].m_limit_except;

	return rangeToString(le.begin(), le.end());
}

#pragma endregion
