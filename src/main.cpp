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
	int fd = initialize_port(8080);

	Poller poller(fd);
	poller.start();

	close(fd);
}

int initialize_port(int port) {
	sockaddr_in server_address = { 0, AF_INET, htons(port), { inet_addr("127.0.0.1") }, { 0 } };
	const int	fd			   = socket(AF_INET, SOCK_STREAM, 0);
	const int	enable		   = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
		fatal_perror("setsockopt");

	if (bind(fd, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address)) < 0)
		fatal_perror("bind");

	if (listen(fd, SOMAXCONN) < 0)
		fatal_perror("listen");
		
	set_fd_nonblocking(fd);

	Poller poller = Poller(fd);
	poller.start();

	std::cout << "LISTEN ON " << port << std::endl;

	return fd;
}
