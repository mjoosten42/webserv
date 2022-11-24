#pragma once

#include "Connection.hpp"
#include "Listener.hpp"
#include "SourceFds.hpp"
#include "stringutils.hpp"

#include <map>
#include <stdlib.h> // exit
#include <sys/poll.h>
#include <vector>

class Poller {
	public:
		void add(const Listener &listener);

		void start();
		void quit();

	private:
		void pollfdEvent();

		void pollIn(pollfd &pollfd);
		bool pollOut(pollfd &pollfd);
		void pollHup(pollfd &pollfd);

		void acceptClient(FD listener_fd);
		void removeClient(FD index);

		size_t clientsIndex();
		size_t sourceFdsIndex();

		std::vector<pollfd>::iterator find(FD fd);

	private:
		static bool m_active;

		std::vector<pollfd>		 m_pollfds;		// the array of pollfd structs
		std::map<FD, Listener>	 m_listeners;	// map server fd with corresponding listener
		std::map<FD, Connection> m_connections; // map client fd to its handler
		SourceFds				 m_sources;		// map source fds to client fds and vice versa.
};
