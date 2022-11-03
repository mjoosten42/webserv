#include "Poller.hpp"

#include "Server.hpp"
#include "SourceFds.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

void Poller::start() {

	LOG(RED "\n----STARTING LOOP----\n" DEFAULT);

	while (true) {

		LOG(RED << std::string(winSize(), '#') << DEFAULT);

		LOG(RED "Clients: " DEFAULT << getPollFdsAsString(clientsIndex(), sourceFdsIndex()));
		// LOG(RED "sourcefds: " DEFAULT << getPollFdsAsString(sourceFdsIndex(), m_pollfds.size()));

		int poll_status = poll(m_pollfds.data(), static_cast<nfds_t>(m_pollfds.size()), -1);
		switch (poll_status) {
			case -1:
				fatal_perror("poll");
			case 0:
				LOG_ERR("Poll returned 0");
				break;
			default:
				pollfdEvent();
		}
	}
}

void Poller::pollfdEvent() {

	// loop over current clients to check if we can read or write
	for (size_t i = clientsIndex(); i < sourceFdsIndex(); i++) {
		pollfd& client = m_pollfds[i];

		// LOG(client.fd << RED " Set: " << getEventsAsString(client.events) << DEFAULT);
		// LOG(client.fd << RED " Get: " << getEventsAsString(client.revents) << DEFAULT);

		unsetFlag(m_pollfds[i].events, POLLOUT);

		if (client.revents & POLLHUP) {
			pollHup(client);
			continue;
		}
		if (client.revents & POLLIN)
			pollIn(client);
		if (client.revents & POLLOUT)
			pollOut(client);
	}

	// loop over sourcefds. If one has POLLIN, set POLLOUT on it's connection
	for (size_t i = sourceFdsIndex(); i < m_pollfds.size(); i++) {
		pollfd& source	  = m_pollfds[i];
		int		client_fd = m_sources.getClientFd(source.fd);

		// LOG(source.fd << RED " Set: " << getEventsAsString(source.events) << DEFAULT);
		// LOG(source.fd << RED " Get: " << getEventsAsString(source.revents) << DEFAULT);

		// If source has POLLIN, set POLLOUT in client
		// However, source will get POLLIN next loop because client will only read next loop, not this one
		// To prevent resetting POLLOUT when the source is done, we unset POLLIN of source for one loop
		if (source.revents & POLLIN) {
			setFlag(find(client_fd)->events, POLLOUT);
			unsetFlag(source.events, POLLIN);
		} else
			setFlag(source.events, POLLIN);
	}

	// Loop over the listening sockets for new clients
	for (size_t i = 0; i < m_listeners.size(); i++)
		if (m_pollfds[i].revents & POLLIN)
			acceptClient(m_pollfds[i].fd);
}

// Let connection read new data
// Add a readfd if asked
void Poller::pollIn(pollfd& client) {
	Connection& conn   = m_connections[client.fd];
	int			readfd = conn.receiveFromClient(client.events);

	if (readfd != -1) { // A response wants to poll on a file/pipe
		pollfd source = { readfd, POLLIN, 0 };

		m_sources.add(source, client.fd);
		m_pollfds.push_back(source);
	}
}

// Let connection send data
// If response if done reading from its readfd, remove it
// If response is done and wants to close the connection, remove the client
void Poller::pollOut(pollfd& client) {
	Connection& conn	  = m_connections[client.fd];
	int			source_fd = conn.sendToClient(client.events);

	if (source_fd != -1) { // returns readfd to close
		m_sources.remove(source_fd);
		m_pollfds.erase(find(source_fd));
	}
	if (conn.wantsClose()) // returns true if clients wants close
		removeClient(client.fd);
}

// We close source fds here because if we received POLLHUP, it means the response hasn't had a chance to close
// Close and remove all source fds connected to client
// Close and remove client
void Poller::pollHup(pollfd& client) {
	std::vector<int> sourcefds = m_sources.getSourceFds(client.fd); // get all sourcef ds dependent on client fd

	for (size_t i = 0; i < sourcefds.size(); i++) {
		int source_fd = sourcefds[i];
		if (close(source_fd) == -1)
			fatal_perror("close");
		m_sources.remove(source_fd);
		m_pollfds.erase(find(source_fd));
	}
	removeClient(client.fd);
}

void Poller::acceptClient(int listener_fd) {
	int				fd		 = accept(listener_fd, NULL, NULL);
	pollfd			client	 = { fd, POLLIN, 0 };
	const Listener *listener = &m_listeners[listener_fd];

	if (fd < 0)
		fatal_perror("accept");

	set_fd_nonblocking(fd);

	m_pollfds.insert(m_pollfds.begin() + sourceFdsIndex(), client); // insert after last client
	m_connections[fd] = Connection(fd, listener);

	LOG(RED "NEW CLIENT: " DEFAULT << fd);
}

void Poller::removeClient(int client_fd) {
	m_pollfds.erase(find(client_fd)); // erase from pollfd vector
	m_connections.erase(client_fd);	  // erase from connection map

	if (close(client_fd) == -1) // close connection
		fatal_perror("close");

	LOG(RED "CLIENT " DEFAULT << client_fd << RED " LEFT" DEFAULT);
}

size_t Poller::clientsIndex() const {
	return m_listeners.size();
}

size_t Poller::sourceFdsIndex() const {
	return m_listeners.size() + m_connections.size();
}

std::vector<pollfd>::iterator Poller::find(int fd) {
	std::vector<pollfd>::iterator it;

	for (it = m_pollfds.begin(); it != m_pollfds.end(); it++)
		if (it->fd == fd)
			return it;
	LOG_ERR("Fd not found in pollfds: " << fd);
	LOG_ERR(RED "Clients: " DEFAULT << getPollFdsAsString(clientsIndex(), sourceFdsIndex()));
	LOG_ERR(RED "sourcefds: " DEFAULT << getPollFdsAsString(sourceFdsIndex(), m_pollfds.size()));
	return m_pollfds.end();
}

std::string Poller::getPollFdsAsString(size_t first, size_t last) const {
	std::string PollFds = "{ ";
	for (; first != last; first++) {
		PollFds += toString(m_pollfds[first].fd);
		if (first + 1 != last)
			PollFds += ",";
		PollFds += " ";
	}
	PollFds += "}";
	return PollFds;
}
