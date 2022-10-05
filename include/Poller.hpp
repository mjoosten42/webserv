#pragma once

#include <map>
#include <poll.h>
#include <vector>

class Server; //  le epic forward declaration

typedef struct pollfd pollfd_t;

class Poller {
	public:
		Poller(std::vector<Server>& servers);
		~Poller();

		void start(); //  starts the polling.

	private:
		void	 setupPollfdsServers();
		pollfd_t create_pollfd(int fd, short events) const;

		void acceptClient(int serverfd);
		bool receiveFromClient(int fd);

	private:
		std::vector<pollfd_t> m_pollfds; //  the array of pollfd structs
		std::vector<Server> & m_servers; //  array of servers
		std::map<int, const Server *>
			m_fdservermap; //  the map which maps the client/server fds with corresponding server
};
