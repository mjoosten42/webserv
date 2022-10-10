#pragma once

#include "Request.hpp"
#include "Response.hpp"

#include <map>
#include <poll.h>
#include <vector>

class Server;
class Handler;

class Poller {
	public:
		Poller(const Server *servers, int size);
		~Poller();

		void start(); //  starts the polling.

	private:
		void acceptClient(int serverfd);
		void removeClient(int fd);
		bool receiveFromClient(int fd);

	private:
		std::vector<pollfd> m_pollfds; //  the array of pollfd structs

		const Server *m_servers; //  array of servers
		std::size_t	  m_nb_servers;

		std::map<int, const Server *> m_fdToServerMap; //  the map which maps the client fds with corresponding server
		std::map<int, Handler>		  m_handlers;	   //  map fd to its handler
};
