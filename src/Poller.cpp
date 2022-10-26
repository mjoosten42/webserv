#include "Poller.hpp"

#include "Server.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

void Poller::start() {

	LOG(RED "\n----STARTING LOOP----\n" DEFAULT);

	while (true) {
		LOG(RED "Clients: " DEFAULT << getPollFdsAsString(clientsIndex(), readFdsIndex()));
		LOG(RED "ReadFds: " DEFAULT << getPollFdsAsString(readFdsIndex(), m_pollfds.size()));

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

	//  loop over current clients to check if we can read or write
	for (size_t i = clientsIndex(); i < readFdsIndex(); i++) {
		pollfd& pollfd = m_pollfds[i];

		LOG(pollfd.fd << RED " Set: " << getEventsAsString(pollfd.events) << DEFAULT);
		LOG(pollfd.fd << RED " Get: " << getEventsAsString(pollfd.revents) << DEFAULT);

		unsetFlag(pollfd.events, POLLOUT);

		if (pollfd.revents & POLLHUP) {
			pollHup(pollfd);
			continue;
		}
		if (pollfd.revents & POLLIN)
			pollIn(pollfd);
		if (pollfd.revents & POLLOUT)
			pollOut(pollfd);
	}

	// loop over readfds
	for (size_t i = readFdsIndex(); i < m_pollfds.size(); i++) {
		if (m_pollfds[i].revents & POLLIN) {
			int		clientfd = getClientFd(m_pollfds[i].fd);
			pollfd& pollfd	 = getPollfd(clientfd);
			setFlag(pollfd.events, POLLOUT);
		}
	}

	//  Loop over the listening sockets for new clients
	for (size_t i = 0; i < m_listeners.size(); i++)
		if (m_pollfds[i].revents & POLLIN)
			acceptClient(m_pollfds[i].fd);
}

// Let connection read new data
// Add a readfd if asked
void Poller::pollIn(pollfd& pollfd) {
	Connection& conn   = m_connections[pollfd.fd];
	int			readfd = conn.receiveFromClient(pollfd.events);

	if (readfd != -1)
		addReadFd(readfd, pollfd.fd);
}

// Let connection send data
// If response if done reading from its readfd, remove it
// If response is done and wants to close the connection, remove the client
void Poller::pollOut(pollfd& pollfd) {
	Connection		   & conn = m_connections[pollfd.fd];
	std::pair<bool, int> ret  = conn.sendToClient(pollfd.events);

	if (ret.second != -1) // returns readfd to close
		removeReadFd(ret.second);
	if (ret.first) // returns true if clients wants close
		removeClient(pollfd.fd);
}

// Close and remove all readfds connected to client
// Close and remove client
void Poller::pollHup(pollfd& pollfd) {
	std::vector<int> readFds = getReadFds(pollfd.fd); // get readfds dependent on client fd

	for (size_t i = 0; i < readFds.size(); i++) {
		if (close(readFds[i]) == -1)
			fatal_perror("close");
		removeReadFd(readFds[i]); // remove readfds
	}
	removeClient(pollfd.fd);
}

void Poller::acceptClient(int listener_fd) {
	int				fd		 = accept(listener_fd, NULL, NULL);
	pollfd			client	 = { fd, POLLIN, 0 };
	const Listener *listener = &m_listeners[listener_fd];

	if (fd < 0)
		fatal_perror("accept");

	set_fd_nonblocking(fd);

	m_pollfds.insert(m_pollfds.begin() + readFdsIndex(), client); // insert after last client
	m_connections[fd] = Connection(fd, listener);

	LOG(RED "NEW CLIENT: " DEFAULT << fd);
}

void Poller::removeClient(int client_fd) {
	size_t index = getPollfdIndex(client_fd);

	m_pollfds.erase(m_pollfds.begin() + index); //  erase from pollfd vector
	m_connections.erase(client_fd);				//  erase from connection map

	if (close(client_fd) == -1) // close connection
		fatal_perror("close");

	LOG(RED "CLIENT " DEFAULT << client_fd << RED " LEFT" DEFAULT);
}

void Poller::addReadFd(int read_fd, int client_fd) {
	pollfd pollfd = { read_fd, POLLIN, 0 };

	m_pollfds.push_back(pollfd); // Add to end
	m_readfds.push_back(std::make_pair(read_fd, client_fd));
}

void Poller::removeReadFd(int read_fd) {
	// Remove read_fd from pollfds
	for (size_t i = readFdsIndex(); i < m_pollfds.size(); i++) {
		if (m_pollfds[i].fd == read_fd) {
			m_pollfds.erase(m_pollfds.begin() + i);
			break;
		}
	}

	// Remove read_fd from readFds
	for (size_t i = 0; i < m_readfds.size(); i++) {
		if (m_readfds[i].first == read_fd) {
			m_readfds.erase(m_readfds.begin() + i);
			break;
		}
	}
}

int Poller::getClientFd(int read_fd) {
	for (size_t i = 0; i < m_readfds.size(); i++)
		if (m_readfds[i].first == read_fd)
			return m_readfds[i].second;
	LOG_ERR("Client for readfd " << read_fd << " not found!");
	LOG_ERR(getReadFdsAsString());
	return -1;
}

std::vector<int> Poller::getReadFds(int client_fd) {
	std::vector<int> fds;

	for (size_t i = 0; i < m_readfds.size(); i++)
		if (m_readfds[i].second == client_fd)
			fds.push_back(m_readfds[i].first);
	return fds;
}

pollfd& Poller::getPollfd(int fd) {
	for (size_t i = 0; i < m_pollfds.size(); i++)
		if (m_pollfds[i].fd == fd)
			return m_pollfds[i];
	LOG_ERR("pollfd " << fd << " not found!");
	LOG_ERR(RED "Clients: " DEFAULT << getPollFdsAsString(0, m_pollfds.size()));
	exit(EXIT_FAILURE);
}

size_t Poller::clientsIndex() {
	return m_listeners.size();
}

size_t Poller::readFdsIndex() {
	return m_listeners.size() + m_connections.size();
}

std::string Poller::getPollFdsAsString(size_t first, size_t last) {
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

std::string Poller::getReadFdsAsString() {
	std::string str = "ReadFds: {";
	for (size_t i = 0; i < m_readfds.size(); i++)
		str += "\n\t{ " + toString(m_readfds[i].first) + ", " + toString(m_readfds[i].second) + " }";
	str += "\n}";
	return str;
}

size_t Poller::getPollfdIndex(int fd) {
	for (size_t i = 0; i < m_pollfds.size(); i++)
		if (fd == m_pollfds[i].fd)
			return i;
	LOG_ERR("pollfd " << fd << " not found!");
	LOG_ERR(RED "Clients: " DEFAULT << getPollFdsAsString(0, m_pollfds.size()));
	return -1;
}
