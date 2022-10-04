#include "Poller.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h> // close

#define BUFSIZE 2048


Poller::Poller(int server_socket) :
	m_servsock(server_socket)
{
    this->m_pollfds = std::vector<pollfd_t>();
}

Poller::~Poller()
{
    
}

pollfd_t Poller::create_pollfd(int fd, short events) const
{
	pollfd_t temp;

	temp.fd = fd;
	temp.events = events;

	return temp;
}

// accepts a client
void Poller::acceptClient()
{
	int clientfd = accept(this->m_pollfds[0].fd, nullptr, nullptr);

	if (clientfd < 0)
	{
		fatal_perror("accept");
	}

	set_fd_nonblocking(clientfd);

	std::cout << "NEW CLIENT\n";

	this->m_pollfds.push_back(create_pollfd(clientfd, POLLIN));
}

// when POLLIN is set, receives a message from the client
// returns true on success, false when the client sents EOF.
bool Poller::receiveFromClient(int fd)
{
	static int	num_recvs = 0;
	char		buf[BUFSIZE];

	bzero(buf, BUFSIZE - 1);

	ssize_t recv_len = recv(fd, buf, BUFSIZE - 1, 0);

	if (recv_len == -1)
	{
		fatal_perror("recv");
	}
	else if (recv_len == 0)
	{
		close(fd);
		return false;
	}
	print(buf);

	std::string number_str = std::to_string(num_recvs);
	std::string	str("HTTP/1.1 200 OK\nCache-Control: no-cache\nServer: libnhttpd\nDate: Wed Jul 4 15:32:03 2012\nConnection: Keep-Alive:\nContent-Type: text/plain\nContent-Length: ");
	str += std::to_string(number_str.length());
	str += "\r\n\r\n";
	str += number_str;
	str += "\r\n\r\n";
	num_recvs++;

	if (send(fd, str.c_str(), str.length(), 0) == -1)
	{
		fatal_perror("send");
	}
	print(buf);
	return true;
}

void Poller::start()
{
	this->m_pollfds.push_back(create_pollfd(this->m_servsock, POLLIN));

	while (true)
	{
		int poll_status = poll(&this->m_pollfds[0], (nfds_t)this->m_pollfds.size(), -1);

		if (poll_status == -1)
		{
			fatal_perror("poll");
		}
		else if (poll_status == 0)
		{
			std::cerr << "No events? wtf??\n";
		}

		// main socket has a new client.
		if (this->m_pollfds[0].revents & POLLIN)
		{
			this->acceptClient();
		}

		// loop through sockets to see if there are new messages
		for (size_t i = 1; i < this->m_pollfds.size(); i++)
		{
			if (this->m_pollfds[i].revents & POLLIN)
			{
				if (this->receiveFromClient(this->m_pollfds[i].fd) == false)
				{
					this->m_pollfds[i] = this->m_pollfds.back();
					this->m_pollfds.pop_back();
					i--;
					std::cout << "CLIENT LEFT\n";
				}
			}
		}
	}
}
