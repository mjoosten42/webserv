#include "Poller.hpp"

#include "FD.hpp"
#include "Server.hpp"
#include "SourceFds.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // pollfd

bool Poller::m_active = false;

void Poller::add(const Listener &listener) {
	FD	   fd	  = listener.getFD();
	pollfd server = { fd, POLLIN, 0 };

	m_pollfds.push_back(server);
	m_listeners[fd] = listener;
}

void Poller::start() {
	m_active = true;

	for (auto &pair : m_listeners)
		pair.second.listen();

	LOG(GREEN "\n----STARTING LOOP----\n" DEFAULT);

	while (m_active) {
		LOG(CYAN << std::string(winSize(), '#') << DEFAULT);
		LOG(CYAN "Servers: " DEFAULT << rangeToString(m_pollfds.begin(), m_pollfds.begin() + clientsIndex()));
		LOG(CYAN "Clients: " DEFAULT << rangeToString(m_pollfds.begin() + clientsIndex(), m_pollfds.end()));

		int poll_status = WS::poll(m_pollfds);
		switch (poll_status) {
			case -1:
				if (errno == EINTR) // SIGCHLD
					continue;
				perror("poll");
			case 0: // Poll is blocking
				m_active = false;
				break;
			default:
				pollfdEvent();
		}
	}

	LOG(GREEN "\n----EXITING LOOP----" DEFAULT);
}

void Poller::quit() {
	m_active = false;
}

void Poller::pollfdEvent() {
	std::vector<int> toAdd;
	size_t			 i = 0; // Use an index because it can't be invalidated

	// Loop over the listening sockets for new clients
	for (; i != clientsIndex(); i++)
		if (m_pollfds[i].revents & POLLIN)
			toAdd.push_back(m_pollfds[i].fd);

	// loop over current clients to check if we can read or write
	for (; i != m_pollfds.size(); i++) {
		pollfd	   &client = m_pollfds[i];
		Connection &conn   = m_connections[client.fd];

		if (client.events)
			LOG(CYAN "Client " DEFAULT << client.fd << CYAN " Set: " DEFAULT << getEventsAsString(client.events));
		if (client.revents)
			LOG(CYAN "Client " DEFAULT << client.fd << CYAN " Get: " DEFAULT << getEventsAsString(client.revents));
	
		client.events = 0;

		if (m_pollfds[i].revents & POLLIN)
			m_pollfds[i].events |= conn.receive();

		if (m_pollfds[i].revents & POLLOUT)
			m_pollfds[i].events |= conn.send();

		if (m_pollfds[i].revents & POLLHUP || conn.wantsClose()) {
			removeClient(m_pollfds[i].fd);
			i--;
			continue;
		}
	}

	for (auto fd : toAdd)
		acceptClient(fd);
}

void Poller::acceptClient(FD listener_fd) {
	sockaddr_in		peer;
	const Listener *listener = &m_listeners[listener_fd];
	FD				fd		 = WS::accept(listener_fd, reinterpret_cast<sockaddr *>(&peer));
	pollfd			client	 = { fd, POLLIN, 0 };

	if (fd < 0)
		return;

	set_fd_nonblocking(fd);

	m_pollfds.insert(m_pollfds.end(), client); // insert after last client
	m_connections[fd] = Connection(fd, listener, this, addressToString(peer.sin_addr.s_addr));

	LOG(CYAN "NEW CLIENT: " DEFAULT << fd);
}

void Poller::removeClient(FD client_fd) {
	m_pollfds.erase(find(client_fd)); // erase from pollfd vector
	m_connections.erase(client_fd);	  // erase from connection map

	LOG(CYAN "CLIENT " DEFAULT << client_fd << CYAN " LEFT" DEFAULT);
}

size_t Poller::clientsIndex() {
	return m_listeners.size();
}

std::vector<pollfd>::iterator Poller::find(FD fd) {
	for (auto it = m_pollfds.begin(); it != m_pollfds.end(); it++)
		if (it->fd == fd)
			return it;
	LOG_ERR("Fd not found in pollfds: " << fd);
	LOG_ERR("Servers: " << rangeToString(m_pollfds.begin(), m_pollfds.end()));
	return m_pollfds.end();
}
