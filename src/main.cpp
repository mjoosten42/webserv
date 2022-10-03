#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include <unistd.h>
#include "utils.hpp"


#define BUFSIZE 1024

int	main() {
	sockaddr_in	addr;
	sockaddr	client;
	socklen_t	tmp = 1;
	char		buf[BUFSIZE];

	int 		fd;
	int			conn;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&addr, sizeof(addr));
	addr.sin_port = htons(8080);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	std::cout << "Now listening on "  << ntohs(addr.sin_port) << std::endl;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp)) < 0)
		perror("setsockopt");
	
	if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
		perror("bind");

	if (listen(fd, 1) < 0)
		perror("listen");

	int	pollfd_len = 100;
	int client_count = 1;

	struct pollfd *fdlist = (struct pollfd *)malloc(pollfd_len * sizeof(struct pollfd));
	fdlist[0].fd = fd;
	fdlist[0].events = POLLIN;

	int n = 0;

	while (1)
	{
		int poll_status = poll(fdlist, (nfds_t)client_count, -1);

		if (poll_status == -1)
		{
			perror("poll");
			return 1;
		}
		else if (poll_status == 0)
		{
			std::cerr << "No events? wtf??\n";
		}

		// main socket has a new client.
		if (fdlist[0].revents & POLLIN)
		{
			conn = accept(fd, &client, &tmp);

			if (conn < 0)
			{
				perror("accept");
			}

			std::cout << "NEW CLIENT\n";
			bzero(buf, BUFSIZE - 1);
			recv(conn, buf, BUFSIZE - 1, 0);
			print(buf);
			std::string	str("HTTP/1.1 200 OK\nCache-Control: no-cache\nServer: libnhttpd\nDate: Wed Jul 4 15:32:03 2012\nConnection: Keep-Alive:\nContent-Type: text/plain\nContent-Length: 6\n\namogus");
			send(conn, str.c_str(), str.length(), 0);
			print(buf);

			fdlist[client_count].fd = conn;
			fdlist[client_count].events = POLLIN;
			client_count++;
		}

		for (int i = 0; i < client_count; i++)
		{
			if (fdlist[i].revents & POLLIN)
			{
				// TODO: Remove socket(when recv returns 0)

				bzero(buf, BUFSIZE - 1);
				recv(conn, buf, BUFSIZE - 1, 0);
				print(buf);

				std::string number_str = std::to_string(n);
				std::string	str("HTTP/1.1 200 OK\nCache-Control: no-cache\nServer: libnhttpd\nDate: Wed Jul 4 15:32:03 2012\nConnection: Keep-Alive:\nContent-Type: text/plain\nContent-Length: ");
				str += std::to_string(number_str.length());
				str += "\r\n\r\n";
				str += number_str;
				str += "\r\n\r\n";
				n++;
				send(conn, str.c_str(), str.length(), 0);
				print(buf);
			}
		}
	}

	close(fd);
}
