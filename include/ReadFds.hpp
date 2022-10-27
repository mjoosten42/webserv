#pragma once

#include <string>
#include <sys/poll.h>
#include <utility>
#include <vector>

class ReadFds {
	public:
		void add(const pollfd& pollfd, int client_fd);
		void remove(int read_fd);

		int				 getClientFd(int read_fd);
		std::vector<int> getReadFds(int client_fd);

		std::string getReadFdsAsString();

	private:
		std::vector<std::pair<int, int> > m_readfds;
};
