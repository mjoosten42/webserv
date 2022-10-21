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

	private:
		void acceptClient(int listener_fd);
		void removeClient(int fd);

	private:
		std::vector<pollfd>		  m_pollfds;	 //  the array of pollfd structs
		std::map<int, Listener>	  m_listeners;	 //  map server fd with corresponding listener
		std::map<int, Connection> m_connections; //  map client fd to its handler
												 // std::map<int, CGI>		m_cgis;
};
