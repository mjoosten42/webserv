#include "SourceFds.hpp"

#include "logger.hpp"
#include "stringutils.hpp"

#include <sys/poll.h>
#include <vector>

void SourceFds::add(const pollfd& pollfd, int client_fd) {
	m_sourceFds.push_back(std::make_pair(pollfd.fd, client_fd));
}

void SourceFds::remove(int source_fd) {
	auto it = m_sourceFds.begin();
	for (; it != m_sourceFds.end(); it++) {
		if (it->first == source_fd) {
			m_sourceFds.erase(it);
			break;
		}
	}
}

int SourceFds::getClientFd(int source_fd) {
	for (auto& sourceFd : m_sourceFds)
		if (sourceFd.first == source_fd)
			return sourceFd.second;
	return -1;
}

std::vector<int> SourceFds::getSourceFds(int client_fd) {
	std::vector<int> fds;

	for (auto& sourceFd : m_sourceFds)
		if (sourceFd.second == client_fd)
			fds.push_back(sourceFd.first);
	return fds;
}

std::string SourceFds::getSourceFdsAsString() {
	std::string str = "SourceFds: {";
	for (auto& sourceFd : m_sourceFds)
		str += "\n\t{ " + toString(sourceFd.first) + ", " + toString(sourceFd.second) + " }";
	str += "\n}";
	return str;
}
