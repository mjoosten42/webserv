#include "Server.hpp"

#include "utils.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> // close

Server::Server() {
	m_fd = -1;
}

Server::Server(int port) {
	//  Specify server socket info: IPv4 protocol family, port in correct endianness, IP address
	sockaddr_in server		= { 0, AF_INET, htons(port), { inet_addr("127.0.0.1") }, { 0 } };

	//  Setup socket_fd: specify domain (IPv4), communication type, and protocol (default for socket)
	m_fd					= socket(AF_INET, SOCK_STREAM, 0);

	//  On socket_fd, applied at socket level (SOL_SOCKET), set option SO_REUSEADDR (allow bind() to reuse local
	//  addresses), to be enabled
	const socklen_t enabled = 1;
	if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0)
		fatal_perror("setsockopt");

	set_fd_nonblocking(m_fd);

	//  "Assign name to socket" = link socket_fd we configured to the server's socket information
	if (bind(m_fd, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0)
		fatal_perror("bind");

	//  Listens on socket, accepting at most 128 connections
	if (listen(m_fd, SOMAXCONN) < 0)
		fatal_perror("listen");

	std::cout << "SERVER " << m_fd << " LISTENING ON " << port << std::endl;
}

//  Note: closing in a copy will cause errors, either don't copy or use reference-counting
Server::~Server() {
	std::cout << "Closing server: " << m_fd << std::endl;
	if (close(m_fd) < 0)
		perror("close");
}

int Server::getFD() const {
	return m_fd;
}
