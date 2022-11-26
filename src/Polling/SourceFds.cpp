#include "SourceFds.hpp"

#include "logger.hpp"
#include "stringutils.hpp"

#include <sys/poll.h>
#include <vector>

void SourceFds::add(const pollfd &pollfd, FD client_fd) {
	m_sourceFds.push_back(std::make_pair(pollfd.fd, client_fd));
}

void SourceFds::remove(FD source_fd) {
	auto it = m_sourceFds.begin();
	for (; it != m_sourceFds.end(); it++) {
		if (it->first == source_fd) {
			m_sourceFds.erase(it);
			return;
		}
	}
	LOG_ERR("Source not found during removal: " << source_fd);
	LOG_ERR(getSourceFdsAsString());
}

FD SourceFds::getClientFd(FD source_fd) {
	for (auto &sourceFd : m_sourceFds)
		if (sourceFd.first == source_fd)
			return sourceFd.second;
	LOG_ERR("Client not found for source " << source_fd);
	LOG_ERR(getSourceFdsAsString());
	return -1;
}

std::vector<FD> SourceFds::getSourceFds(FD client_fd) {
	std::vector<FD> fds;

	for (auto &sourceFd : m_sourceFds)
		if (sourceFd.second == client_fd)
			fds.push_back(sourceFd.first);
	return fds;
}

std::string SourceFds::getSourceFdsAsString() {
	std::string str = "SourceFds: {";
	for (auto &sourceFd : m_sourceFds)
		str += "\n\t{ " + toString(sourceFd.first) + ", " + toString(sourceFd.second) + " }";
	str += "\n}";
	return str;
}
