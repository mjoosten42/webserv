#pragma once

#include "Connection.hpp"
#include "Listener.hpp"

#include <map>
#include <sys/poll.h>
#include <vector>

class Poller {
	public:
		template <typename InputIt>
		Poller(InputIt first, InputIt last) {
			for (; first != last; first++) {
				pollfd client = { first->getFD(), POLLIN, 0 };
				m_pollfds.push_back(client);
				m_listeners[first->getFD()] = *first;
			}
		}

		void start(); //  starts the polling.

		void addReadfd(int readfd, short *events);
		void removeReadfd(int readfd);

		void pollfdEvent();

	private:
		void acceptClient(int listener_fd);
		void removeClient(int fd);

		size_t clientsIndex();
		size_t readFdsIndex();

	private:
		std::vector<pollfd>		  m_pollfds;	 //  the array of pollfd structs
		std::map<int, Listener>	  m_listeners;	 //  map server fd with corresponding listener
		std::map<int, Connection> m_connections; //  map client fd to its handler
		std::map<int, short *>	  m_readfds;	 // map of pollfds that are waiting to receive data from a file/CGI.
												 // these would need to be reset.
};
