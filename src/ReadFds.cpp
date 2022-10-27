#include "ReadFds.hpp"

#include "logger.hpp"
#include "stringutils.hpp"

#include <sys/poll.h>
#include <vector>

void ReadFds::add(const pollfd& pollfd, int client_fd) {
	m_readfds.push_back(std::make_pair(pollfd.fd, client_fd));
}

void ReadFds::remove(int read_fd) {
	for (size_t i = 0; i < m_readfds.size(); i++) {
		if (m_readfds[i].first == read_fd) {
			m_readfds.erase(m_readfds.begin() + i);
			break;
		}
	}
}

int ReadFds::getClientFd(int read_fd) {
	for (size_t i = 0; i < m_readfds.size(); i++)
		if (m_readfds[i].first == read_fd)
			return m_readfds[i].second;
	LOG_ERR("Client for readfd " << read_fd << " not found!");
	LOG_ERR(getReadFdsAsString());
	return -1;
}

std::vector<int> ReadFds::getReadFds(int client_fd) {
	std::vector<int> fds;

	for (size_t i = 0; i < m_readfds.size(); i++)
		if (m_readfds[i].second == client_fd)
			fds.push_back(m_readfds[i].first);
	return fds;
}

std::string ReadFds::getReadFdsAsString() {
	std::string str = "ReadFds: {";
	for (size_t i = 0; i < m_readfds.size(); i++)
		str += "\n\t{ " + toString(m_readfds[i].first) + ", " + toString(m_readfds[i].second) + " }";
	str += "\n}";
	return str;
}
