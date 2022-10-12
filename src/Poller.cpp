#include "Poller.hpp"

#include "Handler.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

//  accepts a new client
void Poller::acceptClient(int server_fd) {
	int			  fd	 = accept(server_fd, NULL, NULL);
	pollfd		  client = { fd, POLLIN | POLLOUT, 0 };
	const Server *server = &m_servers[server_fd];

	if (fd < 0)
		fatal_perror("accept");

	set_fd_nonblocking(fd);

	m_pollfds.push_back(client);
	m_connections[fd] = Connection(fd, server);

	std::cout << "NEW CLIENT: " << fd << std::endl;
}

void Poller::start() {
	while (true) {
		std::cout << RED "\n----STARTING LOOP----\n" DEFAULT;
		//  std::cout << RED "Servers: " DEFAULT
		//  		  << getPollFdsAsString(m_pollfds.begin(), m_pollfds.begin() + m_servers.size());
		std::cout << RED "Clients: " DEFAULT
				  << getPollFdsAsString(m_pollfds.begin() + m_servers.size(), m_pollfds.end());

		int poll_status = poll(m_pollfds.data(), static_cast<nfds_t>(m_pollfds.size()), -1);

		switch (poll_status) {
			case -1:
				fatal_perror("poll");
			case 0:
				std::cerr << "Fds should be blocking: "
						  << getPollFdsAsString(m_pollfds.begin(), m_pollfds.end()) << std::endl;
				break;
			default:
				//  Loop over servers for new clients
				for (size_t i = 0; i < m_servers.size(); i++)
					if (m_pollfds[i].revents & POLLIN)
						acceptClient(m_pollfds[i].fd);

				//  loop over clients for new messages
				for (size_t i = m_servers.size(); i < m_pollfds.size(); i++) {
					Connection& conn = m_connections[m_pollfds[i].fd];

					std::cout << RED << getReventsAsString(m_pollfds[i].revents) << DEFAULT << std::endl;
					
					if (m_pollfds[i].revents & POLLIN)
						conn.receiveFromClient();
					if (m_pollfds[i].revents & POLLOUT)
						conn.sendToClient();
					if (m_pollfds[i].revents & POLLHUP)
						removeClient(m_pollfds[i--].fd);
				}
		}
	}
}

void Poller::removeClient(int fd) {
	for (size_t i = 0; i < m_pollfds.size(); i++)
		if (m_pollfds[i].fd == fd)
			m_pollfds.erase(m_pollfds.begin() + i);

	m_connections.erase(fd);

	if (close(fd) == -1)
		fatal_perror("close");

	std::cout << "CLIENT " << fd << " LEFT\n";
}
