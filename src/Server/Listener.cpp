#include "Listener.hpp"

#include "Server.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <arpa/inet.h>	// inet_addr
#include <fcntl.h>		// fcntl
#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h>

static void throw_perror(const char *msg) {
	perror(msg);
	throw std::invalid_argument(msg);
}

Listener::Listener() {}

Listener::Listener(const std::string &listenAddress, short port): m_listenAddr(listenAddress), m_port(port) {
	setupSocket();
}

void Listener::addServer(const Server &server) {
	m_servers.push_back(server);

	for (auto &name : server.getNames())
		m_hostToServer[name] = &(m_servers.back());
}

const Server &Listener::getServerByHost(const std::string &host) const {
	// try to find it. Otherwise, use the first server.
	auto it = m_hostToServer.find(host);
	if (it != m_hostToServer.end())
		return *(it->second);
	return m_servers.front();
}

void Listener::setupSocket() {

	// Setup socket_fd: specify domain (IPv4), communication type, and	protocol (default for socket)
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_fd == -1)
		throw_perror("socket");

	// On socket_fd, applied at socket level (SOL_SOCKET), set option
	// SO_REUSEADDR (allow bind() to reuse local addresses), to be enabled
	const socklen_t enabled = 1;
	if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0)
		throw_perror("setsockopt");

	if (fcntl(m_fd, F_SETFL, O_NONBLOCK) == -1)
		throw_perror("fcntl");
}

void Listener::listen() {
	sockaddr_in server;										 // Specify server socket info:
	server.sin_family = AF_INET;							 // IPv4 protocol family
	server.sin_port	  = htons(m_port);						 // port in correct endianness
	server.sin_addr	  = { inet_addr(m_listenAddr.c_str()) }; // IP address

	// "Assign name to socket" = link socket_fd we configured to the server'ssocket information
	if (bind(m_fd, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0)
		throw_perror("bind");

	// Listens on socket, accepting at most 128 connections
	if (::listen(m_fd, SOMAXCONN) < 0)
		throw_perror("listen");

	LOG(GREEN "LISTENER " DEFAULT << m_fd << GREEN " LISTENING ON " DEFAULT << m_listenAddr << ":" << m_port);
}

#pragma region getters

int Listener::getFD() const {
	return m_fd;
}

short Listener::getPort() const {
	return m_port;
}

const std::string &Listener::getListenAddr() const {
	return m_listenAddr;
}

#pragma endregion
