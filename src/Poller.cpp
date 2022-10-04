#include "Poller.hpp"

#include "Server.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

#define BUFSIZE 2048

Poller::Poller(std::vector<Server>& servers): m_servers(servers) {
	setupPollfdsServers();
}

Poller::~Poller() {}

void Poller::setupPollfdsServers() {
	for (std::vector<Server>::iterator it = m_servers.begin(); it != m_servers.end(); it++) {
		m_pollfds.push_back(create_pollfd(it->getFD(), POLLIN));
		m_fdservermap[it->getFD()] = &*it;
	}
}

pollfd_t Poller::create_pollfd(int fd, short events) const {
	pollfd_t temp;

	temp.fd		= fd;
	temp.events = events;

	return temp;
}

//  accepts a client
void Poller::acceptClient(int serverfd) {
	int clientfd = accept(serverfd, nullptr, nullptr);

	if (clientfd < 0)
		fatal_perror("accept");

	set_fd_nonblocking(clientfd);

	std::cout << "NEW CLIENT\n";

	m_pollfds.push_back(create_pollfd(clientfd, POLLIN));
	m_fdservermap[clientfd] = m_fdservermap[serverfd];
}

//  when POLLIN is set, receives a message from the client
//  returns true on success, false when the client sents EOF.
bool Poller::receiveFromClient(int fd) {
	static int num_recvs	= 0;
	char	   buf[BUFSIZE] = { 0 };
	ssize_t	   recv_len		= recv(fd, buf, BUFSIZE - 1, 0);

	if (recv_len == -1) {
		fatal_perror("recv");
	} else if (recv_len == 0) {
		close(fd);
		return false;
	}
	print(buf);

	num_recvs++;

	//  TODO; probably some stuff should be delegated to some other class here

	std::string toSend = "server FD: " + std::to_string(m_fdservermap[fd]->getFD());
	toSend += ", num receives: " + std::to_string(num_recvs) + "\n";

	std::string number_str = std::to_string(num_recvs);
	std::string str("HTTP/1.1 200 OK\r\n"
					"Cache-Control: no-cache\r\n"
					"Server: amogus\r\n"
					"Date: Wed Jul 4 15:32:03 2012\r\n"
					"Connection: Keep-Alive:\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Length: ");
	str += std::to_string(toSend.length());
	str += "\r\n\r\n";
	str += toSend;
	str += "\r\n\r\n";

	if (send(fd, str.c_str(), str.length(), 0) == -1)
		fatal_perror("send");
	print(buf);
	return true;
}

void Poller::start() {
	while (true) {
		int poll_status = poll(m_pollfds.data(), static_cast<nfds_t>(m_pollfds.size()), -1);

		if (poll_status == -1)
			fatal_perror("poll");

		else if (poll_status == 0)
			std::cerr << "No events? wtf??\n";

		for (size_t i = 0; i < m_servers.size(); i++)
			if (m_pollfds[i].revents & POLLIN)
				acceptClient(m_pollfds[i].fd);

		//  loop through sockets to see if there are new messages
		for (size_t i = m_servers.size(); i < m_pollfds.size(); i++) {
			if (m_pollfds[i].revents & POLLIN) {
				if (receiveFromClient(m_pollfds[i].fd) == false) {
					m_pollfds[i] = m_pollfds.back();
					m_pollfds.pop_back();
					i--;
					m_fdservermap.erase(m_pollfds[i].fd);
					std::cout << "CLIENT LEFT\n";
				}
			}
		}
	}
}
