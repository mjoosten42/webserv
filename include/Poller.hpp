#pragma once

#include "Request.hpp"
#include "Response.hpp"

#include <map>
#include <poll.h>
#include <vector>

class Server;

struct stuff {
		stuff(): request(-1, NULL), response(-1, NULL), server(NULL) {}
		stuff(int fd, const Server *server): request(fd, server), response(fd, server), server(server) {}

		Request		  request;
		Response	  response;
		const Server *server;
};

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

		std::map<int, const Server *> m_fdToServerMap;	 //  the map which maps the client fds with corresponding server
		std::map<int, stuff>		  m_requestResponse; //  map fd to its request, response and server
};
