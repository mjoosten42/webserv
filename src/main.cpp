#include "Poller.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

int initialize_port(int port);

int main() {
	// Setting up server's listening socket
	int fd = initialize_port(8080);

	Poller poller(fd);
	poller.start();

	close(fd);
}

int initialize_port(int port) {
	//  Specify server socket info: IPv4 protocol family, port in correct endianness, IP address
	sockaddr_in server		= { 0, AF_INET, htons(port), { inet_addr("127.0.0.1") }, { 0 } };

	//  Setup socket_fd: specify domain (IPv4), communication type, and protocol (default for socket)
	const int socket_fd		= socket(AF_INET, SOCK_STREAM, 0);

	//  On socket_fd, applied at socket level (SOL_SOCKET), set option SO_REUSEADDR (allow bind() to reuse local
	//  addresses), to be enabled
	const socklen_t enabled = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0)
		fatal_perror("setsockopt");

	//  "Assign name to socket" = link socket_fd we configured to the server's socket information
	if (bind(socket_fd, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0)
		fatal_perror("bind");

	// Listens on socket, accepting at most 128 connections
	if (listen(socket_fd, SOMAXCONN) < 0)
		fatal_perror("listen");

	std::cout << "LISTENING ON " << port << std::endl;

	set_fd_nonblocking(socket_fd);

	return socket_fd;
}
