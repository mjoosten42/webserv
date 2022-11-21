#pragma once

#include "FD.hpp"

#include <string>
#include <sys/poll.h>
#include <utility>
#include <vector>

class SourceFds {
	public:
		void add(const pollfd &pollfd, FD client_fd);
		void remove(FD source_fd);

		FD				getClientFd(FD source_fd);
		std::vector<FD> getSourceFds(FD client_fd);

		std::string getSourceFdsAsString();

	private:
		std::vector<std::pair<FD, FD> > m_sourceFds;
};
