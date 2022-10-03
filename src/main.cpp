#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.hpp"

int	main() {
	sockaddr_in	addr;
	int 		fd;
	int i = 1;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&addr, sizeof(addr));
	addr.sin_port = htons(8080);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	print(addr.sin_port);


	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &i, sizeof(i)) < 0)
		perror("setsockopt");
	
	if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
		perror("bind");

	if (listen(fd, 3) < 0)
		perror("listen");
}
