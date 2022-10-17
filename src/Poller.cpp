#include "Poller.hpp"

#include "Handler.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

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
				std::cerr << "Fds should be blocking: " << getPollFdsAsString(m_pollfds.begin(), m_pollfds.end())
						  << std::endl;
				break;
			default:
				//  loop over current clients to check if we can read or write
				for (size_t i = m_servers.size(); i < m_pollfds.size(); i++) {
					Connection& conn = m_connections[m_pollfds[i].fd];

					std::cout << RED << m_pollfds[i].fd << ": Events set: " << getEventsAsString(m_pollfds[i].events)
							  << DEFAULT << std::endl;
					std::cout << RED << m_pollfds[i].fd << ": Events get: " << getEventsAsString(m_pollfds[i].revents)
							  << DEFAULT << std::endl;

					if (m_pollfds[i].revents & POLLHUP)
						removeClient(i--);
					if (m_pollfds[i].revents & POLLIN)
						conn.receiveFromClient(m_pollfds[i].events);
					if (m_pollfds[i].revents & POLLOUT)
						conn.sendToClient(m_pollfds[i].events);
				}

				//  Loop over servers for new clients
				//  This is done after clients so new clients don't get polled immediately after accepting
				for (size_t i = 0; i < m_servers.size(); i++)
					if (m_pollfds[i].revents & POLLIN)
						acceptClient(m_pollfds[i].fd);
		}
	}
}

//  accepts a new client
void Poller::acceptClient(int server_fd) {
	int			  fd	 = accept(server_fd, NULL, NULL);
	pollfd		  client = { fd, POLLIN, 0 };
	const Server *server = &m_servers[server_fd];

	if (fd < 0)
		fatal_perror("accept");

	set_fd_nonblocking(fd);

	m_pollfds.push_back(client);
	m_connections[fd] = Connection(fd, server);

	std::cout << "NEW CLIENT: " << fd << std::endl;
}

void Poller::removeClient(int i) {
	int fd = m_pollfds[i].fd;

	m_pollfds.erase(m_pollfds.begin() + i); //  erase from pollfd vector
	m_connections.erase(m_pollfds[i].fd);	//  erase from connection map

	if (close(fd) == -1)
		fatal_perror("close");

	std::cout << "CLIENT " << fd << " LEFT\n";
}
