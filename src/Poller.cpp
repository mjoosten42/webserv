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
		LOG(RED "readfds: " DEFAULT << getPollFdsAsString(readFdsIndex(), m_pollfds.size()));

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
		pollfd	  & pollfd = m_pollfds[i];
		Connection& conn   = m_connections[pollfd.fd];

		LOG(RED "CLIENT: " DEFAULT << pollfd.fd);
		LOG(pollfd.fd << RED ": Events set: " << getEventsAsString(pollfd.events) << DEFAULT);
		LOG(pollfd.fd << RED ": Events get: " << getEventsAsString(pollfd.revents) << DEFAULT);

		unsetFlag(pollfd.events, POLLOUT);

		if (pollfd.revents & POLLHUP) {
			removeClient(i--);
			continue;
		}
		if (pollfd.revents & POLLIN) {
			int readfd = conn.receiveFromClient(pollfd.events);
			if (readfd != -1)
				addReadFd(readfd, pollfd.fd);
		}
		if (pollfd.revents & POLLOUT) {
			std::pair<bool, int> ret = conn.sendToClient(pollfd.events);
			if (ret.second != -1) // returns readfd to close
				removeReadFd(ret.second);
			if (ret.first) // returns true if clients wants close
				removeClient(i--);
		}
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

// TODO: use client_fd
void Poller::removeClient(int index) {
	int fd = m_pollfds[index].fd;

	m_pollfds.erase(m_pollfds.begin() + index); //  erase from pollfd vector
	m_connections.erase(fd);					//  erase from connection map

	std::vector<int> readFds = getReadFds(fd);	// get readfds dependent on client fd
	for (size_t i = 0; i < readFds.size(); i++)
		removeReadFd(readFds[i]);				// remove readfds
	
	if (close(fd) == -1)
		fatal_perror("close");

	LOG(RED "CLIENT " DEFAULT << fd << RED " LEFT" DEFAULT);
}

void Poller::addReadFd(int read_fd, int client_fd) {
	pollfd pollfd = { read_fd, POLLIN, 0 };

	m_pollfds.push_back(pollfd); // Add to end
	m_readfds.push_back(std::make_pair(read_fd, client_fd));
}

void Poller::removeReadFd(int read_fd) {
	for (size_t i = readFdsIndex(); i < m_pollfds.size(); i++) {
		if (m_pollfds[i].fd == read_fd) {
			m_pollfds.erase(m_pollfds.begin() + i);
			break;
		}
	}

	for (size_t i = 0; i < m_readfds.size(); i++) {
		if (m_readfds[i].first == read_fd) {
			m_readfds.erase(m_readfds.begin() + i--);
			break;
		}
	}
}

int Poller::getClientFd(int readfd) {
	for (size_t i = 0; i < m_readfds.size(); i++)
		if (m_readfds[i].first == readfd)
			return m_readfds[i].second;
	LOG_ERR("Client fd not found from readfd: " << readfd);
	LOG_ERR(RED "Readfds - clients: " << getReadFdsAsString());
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
	std::string str = "{";
	for (size_t i = 0; i < m_readfds.size(); i++)
		str += "\n\t{ " + toString(m_readfds[i].first) + ", " + toString(m_readfds[i].second) + " }";
	str += "\n}";
	return str;
}
