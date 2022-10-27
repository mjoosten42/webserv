#pragma once

#include "Connection.hpp"
#include "Listener.hpp"
#include "ReadFds.hpp"
#include "stringutils.hpp"

#include <map>
#include <sys/poll.h>
#include <vector>

class Poller {
	public:
		template <typename InputIt>
		Poller(InputIt first, InputIt last) {
			for (; first != last; first++) {
				pollfd server = { first->getFD(), POLLIN, 0 };
				m_pollfds.push_back(server);
				m_listeners[first->getFD()] = *first;
			}
		}

		void start(); // starts the polling.

	private:
		void pollfdEvent();

		void pollIn(pollfd& pollfd);
		void pollOut(pollfd& pollfd);
		void pollHup(pollfd& pollfd);

		void acceptClient(int listener_fd);
		void removeClient(int index);

		size_t clientsIndex();
		size_t readFdsIndex();

		std::vector<pollfd>::iterator find(int fd);

		std::string getPollFdsAsString(size_t first, size_t last);

	private:
		std::vector<pollfd>		  m_pollfds;	 // the array of pollfd structs
		std::map<int, Listener>	  m_listeners;	 // map server fd with corresponding listener
		std::map<int, Connection> m_connections; // map client fd to its handler
		ReadFds					  m_readfds;	 // map readfds to clientfds and vice versa.
};
