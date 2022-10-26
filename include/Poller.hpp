#pragma once

#include "Connection.hpp"
#include "Listener.hpp"
#include "stringutils.hpp"

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
		void pollfdEvent();

		void acceptClient(int listener_fd);
		void removeClient(int index);

		void addReadFd(int read_fd, int client_fd);
		void removeReadFd(int read_fd);

		int getClientFd(int read_fd);
		std::vector<int> getReadFds(int client_fd);

		pollfd& getPollfd(int fd);

		size_t clientsIndex();
		size_t readFdsIndex();

		std::string getPollFdsAsString(size_t first, size_t last);
		std::string getReadFdsAsString();

	private:
		std::vector<pollfd>				  m_pollfds;	 // the array of pollfd structs
		std::map<int, Listener>			  m_listeners;	 // map server fd with corresponding listener
		std::map<int, Connection>		  m_connections; // map client fd to its handler
		std::vector<std::pair<int, int> > m_readfds;	 // map readfds to clientfds.
		// std::map<int, int>		  m_readfds;	 // map of pollfds that are waiting to receive data from a file/CGI.
		// these would need to be reset.
};
