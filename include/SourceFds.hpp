#pragma once

#include <string>
#include <sys/poll.h>
#include <utility>
#include <vector>

class SourceFds {
	public:
		void add(const pollfd& pollfd, int client_fd);
		void remove(int source_fd);

		int				 getClientFd(int source_fd);
		std::vector<int> getSourceFds(int client_fd);

		std::string getSourceFdsAsString();

	private:
		std::vector<std::pair<int, int> > m_sourceFds;
};
