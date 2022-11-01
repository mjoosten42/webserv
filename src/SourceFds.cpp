#include "SourceFds.hpp"

#include "logger.hpp"
#include "stringutils.hpp"

#include <sys/poll.h>
#include <vector>

void SourceFds::add(const pollfd& pollfd, int client_fd) {
	m_sourceFds.push_back(std::make_pair(pollfd.fd, client_fd));
}

void SourceFds::remove(int source_fd) {
	for (size_t i = 0; i < m_sourceFds.size(); i++) {
		if (m_sourceFds[i].first == source_fd) {
			m_sourceFds.erase(m_sourceFds.begin() + i);
			break;
		}
	}
}

int SourceFds::getClientFd(int source_fd) {
	for (size_t i = 0; i < m_sourceFds.size(); i++)
		if (m_sourceFds[i].first == source_fd)
			return m_sourceFds[i].second;
	LOG_ERR("Client for source fd " << source_fd << " not found!");
	LOG_ERR(getSourceFdsAsString());
	return -1;
}

std::vector<int> SourceFds::getSourceFds(int client_fd) {
	std::vector<int> fds;

	for (size_t i = 0; i < m_sourceFds.size(); i++)
		if (m_sourceFds[i].second == client_fd)
			fds.push_back(m_sourceFds[i].first);
	return fds;
}

std::string SourceFds::getSourceFdsAsString() {
	std::string str = "SourceFds: {";
	for (size_t i = 0; i < m_sourceFds.size(); i++)
		str += "\n\t{ " + toString(m_sourceFds[i].first) + ", " + toString(m_sourceFds[i].second) + " }";
	str += "\n}";
	return str;
}
