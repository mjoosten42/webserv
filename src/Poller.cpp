#include "Poller.hpp"

#include "Server.hpp"
#include "SourceFds.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // pollfd

bool Poller::m_active = false;

void Poller::add(const Listener& listener) {
	int	   fd	  = listener.getFD();
	pollfd server = { fd, POLLIN, 0 };

	m_pollfds.push_back(server);
	m_listeners[fd] = listener;
}

void Poller::start() {
	m_active = true;

	LOG(RED "\n----STARTING LOOP----\n" DEFAULT);

	while (m_active) {

		LOG(RED << std::string(winSize(), '#') << DEFAULT);
		LOG(RED "Clients: " DEFAULT << getPollFdsAsString(clientsIndex(), sourceFdsIndex()));
		// LOG(RED "sourcefds: " DEFAULT << getPollFdsAsString(sourceFdsIndex(), m_pollfds.size()));

		int poll_status = WS::poll(m_pollfds);
		switch (poll_status) {
			case -1:
				if (errno == EINTR) // SIGCHLD
					continue;
				exit(EXIT_FAILURE); // TODO: throw?
			case 0:					// Poll is blocking
				break;
			default:
				pollfdEvent();
		}
	}

	LOG(RED "\n----EXITING LOOP----" DEFAULT);
}

void Poller::quit() {
	m_active = false;
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
// Add a source_fd if asked
void Poller::pollIn(pollfd& client) {
	Connection& conn	  = m_connections[client.fd];
	int			source_fd = conn.receive(client.events);

	if (source_fd != -1) { // A response wants to poll on a file/pipe
		pollfd source = { source_fd, POLLIN, 0 };

		m_sources.add(source, client.fd);
		m_pollfds.push_back(source);
	}
}

// Let connection send data
// If response is done reading from its source_fd, remove it
// If response is done and wants to close the connection, remove the client
void Poller::pollOut(pollfd& client) {
	Connection& conn	  = m_connections[client.fd];
	int			source_fd = conn.send(client.events);

	if (source_fd != -1) { // returns source_fd to close
		m_sources.remove(source_fd);
		m_pollfds.erase(find(source_fd));
	}
	if (conn.wantsClose())
		removeClient(client.fd);
}

// Remove all source fds connected to client
// Remove client
void Poller::pollHup(pollfd& client) {
	std::vector<int> sourcefds = m_sources.getSourceFds(client.fd); // get all source fds dependent on client fd

	for (size_t i = 0; i < sourcefds.size(); i++) {
		int source_fd = sourcefds[i];

		m_sources.remove(source_fd);
		m_pollfds.erase(find(source_fd));
	}
	removeClient(client.fd);
}

void Poller::acceptClient(int listener_fd) {
	const Listener *listener = &m_listeners[listener_fd];
	sockaddr_in		peer	 = { sizeof(sockaddr_in), AF_INET, 0, { 0 }, { 0 } };
	int				fd		 = WS::accept(listener_fd, reinterpret_cast<sockaddr *>(&peer));
	pollfd			client	 = { fd, POLLIN, 0 };

	if (fd < 0)
		return;

	m_pollfds.insert(m_pollfds.begin() + sourceFdsIndex(), client); // insert after last client
	m_connections[fd] = Connection(fd, listener, addressToString(peer.sin_addr.s_addr));

	LOG(RED "NEW CLIENT: " DEFAULT << fd);
}

void Poller::removeClient(int client_fd) {
	m_pollfds.erase(find(client_fd)); // erase from pollfd vector
	m_connections.erase(client_fd);	  // erase from connection map

	LOG(RED "CLIENT " DEFAULT << client_fd << RED " LEFT" DEFAULT);
}

size_t Poller::clientsIndex() const {
	return m_listeners.size();
}

size_t Poller::sourceFdsIndex() const {
	return m_listeners.size() + m_connections.size();
}

std::vector<pollfd>::iterator Poller::find(int fd) {
	for (auto it = m_pollfds.begin(); it != m_pollfds.end(); it++)
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
