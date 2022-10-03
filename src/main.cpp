#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include <unistd.h>
#include "utils.hpp"


#define BUFSIZE 2048

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

	std::vector<struct pollfd> fdlist = std::vector<struct pollfd>();

	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;

	fdlist.push_back(pfd);

	int n = 0;

	while (1)
	{
		int poll_status = poll(&fdlist[0], (nfds_t)fdlist.size(), -1);

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
				return 1;
			}

			std::cout << "NEW CLIENT\n";

			pfd.fd = conn;
			pfd.events = POLLIN;
			fdlist.push_back(pfd);
		}

		for (size_t i = 1; i < fdlist.size(); i++)
		{
			if (fdlist[i].revents & POLLIN)
			{
				bzero(buf, BUFSIZE - 1);

				ssize_t recv_len = recv(fdlist[i].fd, buf, BUFSIZE - 1, 0);

				if (recv_len == -1)
				{
					perror("recv");
					return 1;
				}
				else if (recv_len == 0)
				{
					close(fdlist[i].fd);
					fdlist[i] = fdlist.back();
					i--;
					fdlist.pop_back();
					std::cout << "CLIENT LEFT\n";
					continue;
				}
				print(buf);

				std::string number_str = std::to_string(n);
				std::string	str("HTTP/1.1 200 OK\nCache-Control: no-cache\nServer: libnhttpd\nDate: Wed Jul 4 15:32:03 2012\nConnection: Keep-Alive:\nContent-Type: text/plain\nContent-Length: ");
				str += std::to_string(number_str.length());
				str += "\r\n\r\n";
				str += number_str;
				str += "\r\n\r\n";
				n++;

				if (send(fdlist[i].fd, str.c_str(), str.length(), 0) == -1)
				{
					perror("send");
					return 1;
				}
				print(buf);
			}
		}
	}

	close(fd);
}
