#pragma once

#include "Connection.hpp"
#include "Listener.hpp"
#include "SourceFds.hpp"
#include "stringutils.hpp"

#include <map>
#include <sys/poll.h>
#include <vector>

class Poller {
	public:
		Poller();
		~Poller();

		void add(const Listener& listener);

		void start();
		void quit();

	private:
		void pollfdEvent();

		void pollIn(pollfd& pollfd);
		void pollOut(pollfd& pollfd);
		void pollHup(pollfd& pollfd);

		void acceptClient(int listener_fd);
		void removeClient(int index);

		size_t clientsIndex() const;
		size_t sourceFdsIndex() const;

		std::vector<pollfd>::iterator find(int fd);

		std::string getPollFdsAsString(size_t first, size_t last) const;

	private:
		static bool m_active;

		std::vector<pollfd>		  m_pollfds;	 // the array of pollfd structs
		std::map<int, Listener>	  m_listeners;	 // map server fd with corresponding listener
		std::map<int, Connection> m_connections; // map client fd to its handler
		SourceFds				  m_sources;	 // map source fds to client fds and vice versa.
};
