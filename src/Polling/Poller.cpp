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

	LOG(GREEN "\n----STARTING LOOP----\n" DEFAULT);

	while (m_active) {
		LOG(CYAN << std::string(winSize(), '#') << DEFAULT);
		LOG(CYAN "Servers: " DEFAULT << rangeToString(m_pollfds.begin(), m_pollfds.begin() + clientsIndex()));
		LOG(CYAN "Clients: " DEFAULT << rangeToString(m_pollfds.begin() + clientsIndex(),
													  m_pollfds.begin() + sourceFdsIndex()));
		LOG(CYAN "sourcefds: " DEFAULT << rangeToString(m_pollfds.begin() + sourceFdsIndex(), m_pollfds.end()));

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
	size_t i = 0; // Use an index because it can't be invalidated

	// Loop over the listening sockets for new clients
	for (; i != clientsIndex(); i++)
		if (m_pollfds[i].revents & POLLIN)
			acceptClient(m_pollfds[i].fd);

	// loop over current clients to check if we can read or write
	for (; i != sourceFdsIndex(); i++) {
		pollfd &client = m_pollfds[i];

		unsetFlag(client.events, POLLOUT);

		if (client.revents)
			LOG(CYAN "Client " DEFAULT << client.fd << CYAN " Get: " DEFAULT << getEventsAsString(client.revents));

		if (client.revents & POLLHUP) {
			pollHup(client);
			i--;
			continue;
		}
		if (client.revents & POLLIN)
			pollIn(client);
		if (client.revents & POLLOUT)
			if (pollOut(client))
				i--;
	}

	// loop over sourcefds. If one has POLLIN, set POLLOUT on it's connection
	for (; i != m_pollfds.size(); i++) {
		pollfd &source	  = m_pollfds[i];
		FD		client_fd = m_sources.getClientFd(source.fd);

		if (source.revents)
			LOG(CYAN "Source " DEFAULT << source.fd << CYAN " Get: " DEFAULT << getEventsAsString(source.revents));

		// If source has POLLIN, set POLLOUT in client
		// However, source will get POLLIN next loop because client will only read next loop, not this one
		// To prevent resetting POLLOUT when the source is done, we unset POLLIN of source for one loop
		if (source.revents & POLLIN) {
			setFlag(find(client_fd)->events, POLLOUT);
			unsetFlag(source.events, POLLIN);
		} else
			setFlag(source.events, POLLIN);
	}
}

void Poller::pollIn(pollfd &client) {
	Connection &conn	  = m_connections[client.fd];
	FD			source_fd = conn.receive(client.events);

	if (source_fd != -1) { // A response wants to poll on a pipe
		pollfd source = { source_fd, POLLIN, 0 };

		m_sources.add(source, client.fd);
		m_pollfds.push_back(source);
	}
}

bool Poller::pollOut(pollfd &client) {
	Connection &conn	  = m_connections[client.fd];
	FD			source_fd = conn.send(client.events);
	bool		closed	  = false;

	if (source_fd != -1) { // A response wants to close its source_fd
		m_sources.remove(source_fd);
		m_pollfds.erase(find(source_fd));
	}
	if (conn.wantsClose()) {
		removeSources(client.fd);
		removeClient(client.fd);
		closed = true;
	}
	return closed;
}

void Poller::pollHup(pollfd &client) {
	removeSources(client.fd);
	removeClient(client.fd);
}

void Poller::removeSources(FD client) {
	std::vector<FD> sourcefds = m_sources.getSourceFds(client); // get all source fds dependent on client fd

	for (auto &source_fd : sourcefds) {
		m_pollfds.erase(find(source_fd));
		m_sources.remove(source_fd);
	}
}

void Poller::acceptClient(FD listener_fd) {
	const Listener *listener = &m_listeners[listener_fd];
	sockaddr_in		peer	 = { sizeof(sockaddr_in), AF_INET, 0, { 0 }, { 0 } };
	FD				fd		 = WS::accept(listener_fd, reinterpret_cast<sockaddr *>(&peer));
	pollfd			client	 = { fd, POLLIN, 0 };

	if (fd < 0)
		return;

	set_fd_nonblocking(fd);

	m_pollfds.insert(m_pollfds.begin() + sourceFdsIndex(), client); // insert after last client
	m_connections[fd] = Connection(fd, listener, addressToString(peer.sin_addr.s_addr));

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

size_t Poller::sourceFdsIndex() {
	return m_listeners.size() + m_connections.size();
}

std::vector<pollfd>::iterator Poller::find(FD fd) {
	for (auto it = m_pollfds.begin(); it != m_pollfds.end(); it++)
		if (it->fd == fd)
			return it;
	LOG_ERR("Fd not found in pollfds: " << fd);
	LOG_ERR("Servers: " << rangeToString(m_pollfds.begin(), m_pollfds.begin() + clientsIndex()));
	LOG_ERR("Clients: " << rangeToString(m_pollfds.begin() + clientsIndex(), m_pollfds.begin() + sourceFdsIndex()));
	LOG_ERR("Sources: " << rangeToString(m_pollfds.begin() + sourceFdsIndex(), m_pollfds.end()));
	LOG_ERR(m_sources.getSourceFdsAsString());
	return m_pollfds.end();
}
