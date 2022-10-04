#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include <unistd.h>


#include "utils.hpp"
#include "Poller.hpp"

int	main() {
	sockaddr_in	server_address;
	socklen_t	tmp = 1;
	int 		fd;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&server_address, sizeof(server_address));
	server_address.sin_port = htons(8080); // Specify port, htons Tranlsates endianness.
	server_address.sin_family = AF_INET; // Specify IPv4 protocol
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Specify localhost as 

	std::cout << "Now listening on "  << ntohs(server_address.sin_port) << std::endl;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp)) < 0)
		perror("setsockopt");
	
	if (bind(fd, (sockaddr *)&server_address, sizeof(server_address)) < 0)
		perror("bind");

	if (listen(fd, 1) < 0)
		perror("listen");

	set_fd_nonblocking(fd);

	Poller poller = Poller(fd);
	poller.start();

	close(fd);
}
