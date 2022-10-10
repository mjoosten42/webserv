#include "Poller.hpp"

#include "GetStaticFileTransfer.hpp"
#include "Handler.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

#define BUFSIZE 2048

//  accepts a client
void Poller::acceptClient(int server_fd) {
	int			  fd	 = accept(server_fd, NULL, NULL);
	pollfd		  client = { fd, POLLIN, 0 };
	const Server *server = &m_servers[server_fd];

	if (fd < 0)
		fatal_perror("accept");

	set_fd_nonblocking(fd);

	m_pollfds.push_back(client);
	m_handlers[fd] = Handler(fd, server);

	std::cout << "NEW CLIENT: " << fd << std::endl;
}

//  when POLLIN is set, receives a message from the client
//  returns true on success, false when the client sents EOF.
bool Poller::receiveFromClient(int fd) {
	static char buf[BUFSIZE];
	ssize_t		recv_len = recv(fd, buf, BUFSIZE - 1, 0);

	buf[recv_len]		 = 0;

	if (recv_len == -1)
		fatal_perror("recv");
	else if (recv_len == 0)
		return false;

	m_handlers[fd].m_request.add(buf);

	if (recv_len != BUFSIZE - 1)
		m_handlers[fd].m_request.stringToData();

	handleGetWithStaticFile(fd, m_handlers[fd].m_request.getLocation());

	return true;
}

void Poller::start() {
	while (true) {
		std::cout << "\n----STARTING LOOP----\n";

		printFds(m_pollfds.begin(), m_pollfds.end());

		int poll_status = poll(m_pollfds.data(), static_cast<nfds_t>(m_pollfds.size()), -1);

		if (poll_status == -1)
			fatal_perror("poll");

		else if (poll_status == 0)
			std::cerr << "No events? wtf??\n";

		//  Loop over servers for new clients
		for (size_t i = 0; i < m_servers.size(); i++)
			if (m_pollfds[i].revents & POLLIN)
				acceptClient(m_pollfds[i].fd);

		//  loop over clients for new messages
		for (size_t i = m_servers.size(); i < m_pollfds.size(); i++) {
			if (m_pollfds[i].revents & POLLIN)
				if (!receiveFromClient(m_pollfds[i].fd))
					removeClient(m_pollfds[i--].fd);
			if (m_pollfds[i].revents & POLLHUP)
				removeClient(m_pollfds[i--].fd);
		}
	}
}

void Poller::removeClient(int fd) {
	for (size_t i = 0; i < m_pollfds.size(); i++)
		if (m_pollfds[i].fd == fd)
			m_pollfds.erase(m_pollfds.begin() + i);

	m_handlers.erase(fd);

	std::cout << "CLIENT " << fd << " LEFT\n";
}
