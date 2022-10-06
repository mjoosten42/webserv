#pragma once

#include <map>
#include <poll.h>
#include <vector>

class Server;

class Poller {
	public:
		Poller(Server *servers, int size);
		~Poller();

		void start(); //  starts the polling.

	private:
		pollfd create_pollfd(int fd, short events) const;

		void acceptClient(int serverfd);
		bool receiveFromClient(int fd);

	private:
		std::vector<pollfd> m_pollfds; //  the array of pollfd structs
		const Server	   *m_servers; //  array of servers
		std::size_t			m_nb_servers;
		std::map<int, const Server *>
			m_fdservermap; //  the map which maps the client/server fds with corresponding server
};
