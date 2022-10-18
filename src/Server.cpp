#include "Server.hpp"

#include "defines.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> // close

Location::Location() {
	m_location			   = "/";
	m_client_max_body_size = -1;
	m_limit_except		   = "";
	m_redirect			   = "";
	m_auto_index		   = false;
	m_root				   = "html";
}

Location::Location(t_block_directive *constructor_specs, Server *parent) {
	std::string val_from_config;

	m_location		= "";
	val_from_config = constructor_specs->additional_params;
	if (!val_from_config.empty())
		m_location = val_from_config;

	m_client_max_body_size = parent->m_client_max_body_size;
	val_from_config		   = constructor_specs->fetch_simple("client_max_body_size");
	if (!val_from_config.empty())
		m_client_max_body_size = stoi(val_from_config);

	m_limit_except	= "";
	val_from_config = constructor_specs->fetch_simple("limit_except");
	if (!val_from_config.empty())
		m_limit_except = val_from_config;

	m_redirect = ""; //  Not a standard Nginx config param

	m_auto_index	= false;
	val_from_config = constructor_specs->fetch_simple("autoindex");
	if (!val_from_config.empty())
		m_auto_index = (val_from_config.compare("on") == 0 ? true : false);

	m_root			= parent->m_root;
	val_from_config = constructor_specs->fetch_simple("root");
	if (!val_from_config.empty())
		m_root = val_from_config;
}

Server::Server(): m_fd(-1) {
	m_host		 = "localhost";
	m_port		 = 8080;
	m_name		 = "webserv.com"; //  Nginx default = ""
	m_root		 = "html";
	m_error_page = "404.html";

	Location location = Location();
	m_locations.push_back(location);
}

void Server::setupSocket() {
	//  SET UP SOCKET //

	//  Specify server socket info: IPv4 protocol family, port in correct
	//	endianness, IP address
	sockaddr_in server = { 0, AF_INET, htons(m_port), { inet_addr(m_host.c_str()) }, { 0 } };

	//  Setup socket_fd: specify domain (IPv4), communication type, and
	//	protocol (default for socket)
	m_fd = socket(AF_INET, SOCK_STREAM, 0);

	//  On socket_fd, applied at socket level (SOL_SOCKET), set option
	//  SO_REUSEADDR (allow bind() to reuse local addresses), to be enabled
	const socklen_t enabled = 1;
	if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0)
		fatal_perror("setsockopt");

	set_fd_nonblocking(m_fd);

	//  "Assign name to socket" = link socket_fd we configured to the server's
	//	socket information
	if (bind(m_fd, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0)
		fatal_perror("bind");

	//  Listens on socket, accepting at most 128 connections
	if (listen(m_fd, SOMAXCONN) < 0)
		fatal_perror("listen");

	std::cout << RED "SERVER " << m_fd;
	if (!m_name.empty())
		std::cout << " " << m_name;
	std::cout << " LISTENING ON " << m_port << DEFAULT "\n";
}

Server::Server(std::vector<Server>& servers, t_block_directive *constructor_specs) {
	//  INITIALISE MEMBER VARIABLES //
	m_host = "127.0.0.1"; //	The only address we handle requests on is localhost

	std::string val_from_config;
	m_port			= 8000; //	Nginx default is 80 if super user, otherwise 8000
	val_from_config = constructor_specs->fetch_simple("listen");
	if (!val_from_config.empty())
		m_port = stoi(val_from_config);

	m_name			= ""; //  Nginx default: ""
	val_from_config = constructor_specs->fetch_simple("server_name");
	if (!val_from_config.empty())
		m_name = val_from_config;

	m_root			= "html"; //	Nginx default: "html"
	val_from_config = constructor_specs->fetch_simple("root");
	if (!val_from_config.empty())
		m_root = val_from_config;

	m_error_page	= "404.html"; //	Our default 404 error page
	val_from_config = constructor_specs->fetch_simple("error_page");
	if (!val_from_config.empty())
		m_error_page = val_from_config; //  TODO: Check if error page is valid.

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

	//  Check if other servers have the same hostname and port. Otherwise, create the socket.
	//  NOOO! YOU CAN'T USE LOOPS DURING CONFIGURATION PARSING!! THEY'RE INEFFICIENT AND YOU SHOULD USE A MAP NOOOOOOO
	for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); it++) {
		if (it->getHost() == m_host && it->getPort() == m_port) {
			m_fd = it->getFD();
			return;
		}
	}
	setupSocket();
}

#pragma region getters

int Server::getFD() const {
	return m_fd;
}

short Server::getPort() const {
	return m_port;
}

//  returns the host of the server. That is, not the server name, but the listening address.
const std::string& Server::getHost() const {
	return m_host;
}

const std::string& Server::getName() const {
	return m_name;
}

#pragma endregion
