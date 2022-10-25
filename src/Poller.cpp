#include "Poller.hpp"

#include "Server.hpp"
#include "defines.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

void Poller::start() {
	LOG(RED "\n----STARTING LOOP----\n" DEFAULT);
	while (true) {
		LOG(RED "Clients: " DEFAULT << getPollFdsAsString(m_pollfds.begin() + m_listeners.size(), m_pollfds.end()));

		int poll_status = poll(m_pollfds.data(), static_cast<nfds_t>(m_pollfds.size()), -1);

		switch (poll_status) {
			case -1:
				fatal_perror("poll");
			case 0:
				LOG_ERR("Poll returned 0");
				break;
			default:
				// TODO: this is an ugly structure
				// unset all connecton pollouts
				for (size_t i = m_listeners.size(); i < m_pollfds.size() - m_readfds.size(); i++)
					unsetFlag(m_pollfds[i].events, POLLOUT);

				// loop over readfds
				for (size_t i = m_pollfds.size() - m_readfds.size(); i < m_pollfds.size(); i++) {
					if (m_pollfds[i].revents & POLLIN) {
						// does this dereference work?
						setFlag(*m_readfds[m_pollfds[i].fd], POLLOUT);
					}
				}

				//  loop over current clients to check if we can read or write
				for (size_t i = m_listeners.size(); i < m_pollfds.size() - m_readfds.size(); i++) {
					Connection& conn = m_connections[m_pollfds[i].fd];

					unsetFlag(m_pollfds[i].events, POLLOUT);

					LOG(RED "CLIENT: " DEFAULT << m_pollfds[i].fd);
					LOG(m_pollfds[i].fd << RED ": Events set: " << getEventsAsString(m_pollfds[i].events) << DEFAULT);
					LOG(m_pollfds[i].fd << RED ": Events get: " << getEventsAsString(m_pollfds[i].revents) << DEFAULT);

					if (m_pollfds[i].revents & POLLHUP)
						removeClient(i--);
					if (m_pollfds[i].revents & POLLIN)
						conn.receiveFromClient(m_pollfds[i].events);
					if (m_pollfds[i].revents & POLLOUT)
						if (conn.sendToClient(m_pollfds[i].events)) // returns true if clients wants close
							removeClient(i--);
				}

				//  Loop over the listening sockets for new clients
				for (size_t i = 0; i < m_listeners.size(); i++)
					if (m_pollfds[i].revents & POLLIN)
						acceptClient(m_pollfds[i].fd);
		}
	}
}

//  accepts a new client
void Poller::acceptClient(int listener_fd) {
	int				fd		 = accept(listener_fd, NULL, NULL);
	pollfd			client	 = { fd, POLLIN, 0 };
	const Listener *listener = &m_listeners[listener_fd];

	if (fd < 0)
		fatal_perror("accept");

	set_fd_nonblocking(fd);

	m_pollfds.push_back(client);
	m_connections[fd] = Connection(fd, listener);

	LOG(RED "NEW CLIENT: " DEFAULT << fd);
}

void Poller::removeClient(int i) {
	int fd = m_pollfds[i].fd;

	m_pollfds.erase(m_pollfds.begin() + i); //  erase from pollfd vector
	m_connections.erase(fd);				//  erase from connection map

	if (close(fd) == -1)
		fatal_perror("close");

	LOG(RED "CLIENT " DEFAULT << fd << RED " LEFT" DEFAULT);
}

void Poller::addReadfd(int readfd, short *events) {
	pollfd pfd		  = { readfd, POLLIN, 0 };
	m_readfds[readfd] = events;
	m_pollfds.push_back(pfd);
}

void Poller::removeReadfd(int readfd) {
	for (size_t i = m_pollfds.size() - m_readfds.size(); i < m_pollfds.size(); i++) {
		if (m_pollfds[i].fd == readfd) {
			m_pollfds.erase(m_pollfds.begin() + i);
			break;
		}
	}
	m_readfds.erase(readfd);
}
