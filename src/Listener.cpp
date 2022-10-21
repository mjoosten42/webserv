#include "Listener.hpp"

#include "Server.hpp"
#include "defines.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

Listener::Listener(): m_fd(-1), m_listenAddr(""), m_port(-1) {}

Listener::Listener(const std::string& listenAddress, short port): m_fd(-1), m_listenAddr(listenAddress), m_port(port) {
	setupSocket();
}

Listener::~Listener() {}

void Listener::addServer(const Server *server) {
	m_servers.push_back(server);

	const std::vector<std::string>& names = server->getNames();

	std::vector<std::string>::const_iterator it;
	for (it = names.begin(); it != names.end(); it++)
		m_hostToServer[*it] = server;
}

const Server *Listener::getServerByHost(const std::string& host) const {
	//  try to find it. Otherwise, use the first server.
	std::map<std::string, const Server *>::const_iterator it = m_hostToServer.find(host);
	if (it != m_hostToServer.end())
		return it->second;
	return m_servers.front();
}

void Listener::setupSocket() {
	//  SET UP SOCKET //

	//  Specify server socket info: IPv4 protocol family, port in correct
	//	endianness, IP address
	sockaddr_in server = { 0, AF_INET, htons(m_port), { inet_addr(m_listenAddr.c_str()) }, { 0 } };

	//  Setup socket_fd: specify domain (IPv4), communication type, and
	//	protocol (default for socket)
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_fd == -1)
		fatal_perror("socket");

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

	std::cout << RED "\"LISTENER\" " << m_fd;
	std::cout << " LISTENING ON " << m_listenAddr << ":" << m_port << DEFAULT "\n";
}

#pragma region getters

int Listener::getFD() const {
	return m_fd;
}

short Listener::getPort() const {
	return m_port;
}

const std::string& Listener::getListenAddr() const {
	return m_listenAddr;
}

#pragma endregion
