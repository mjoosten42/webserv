#include "Poller.hpp"

#include "Response.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

#define BUFSIZE 2048

Poller::Poller(Server *servers, int size): m_servers(servers), m_nb_servers(size) {
	for (const Server *it = m_servers; it != m_servers + size; it++) {
		pollfd client = { it->getFD(), POLLIN, 0 };
		m_pollfds.push_back(client);
		m_fdservermap[it->getFD()] = &*it;
	}
}

Poller::~Poller() {}

pollfd Poller::create_pollfd(int fd, short events) const {
	pollfd temp;

	temp.fd		= fd;
	temp.events = events;

	return temp;
}

//  accepts a client
void Poller::acceptClient(int serverfd) {
	int clientfd = accept(serverfd, NULL, NULL);

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
	Response response;

	std::string bodyContent = "server FD: " + std::to_string(m_fdservermap[fd]->getFD());
	bodyContent += ", num receives: " + std::to_string(num_recvs) + "\n";

	response.m_statusLine = "HTTP/1.1 200 OK";

	response.m_header.push_back("Cache-Control: no-cache");
	response.m_header.push_back("Server: amogus");
	response.m_header.push_back("Date: Wed Jul 4 15:32:03 2012");
	response.m_header.push_back("Connection: Keep-Alive:");
	response.m_header.push_back("Content-Type: text/plain");
	response.m_header.push_back("Content-Length: " + std::to_string(bodyContent.length()));

	response.m_body.push_back(bodyContent);

	//  std::string number_str = std::to_string(num_recvs); // WAS UNUSED.

	if (send(fd, response.getFormattedResponse().c_str(), response.getFormattedResponse().length(), 0) == -1)
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

		for (size_t i = 0; i < m_nb_servers; i++)
			if (m_pollfds[i].revents & POLLIN)
				acceptClient(m_pollfds[i].fd);

		//  loop through sockets to see if there are new messages
		for (size_t i = m_nb_servers; i < m_pollfds.size(); i++) {
			if (m_pollfds[i].revents & POLLIN) {
				if (receiveFromClient(m_pollfds[i].fd) == false) {
					std::cout << "CLIENT " << m_pollfds[i].fd << " LEFT\n";
					m_fdservermap.erase(m_pollfds[i].fd);
					m_pollfds[i] = m_pollfds.back();
					m_pollfds.pop_back();
					i--;
				}
			}
		}

		print(m_pollfds);
	}
}
