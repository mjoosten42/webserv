#pragma once

#include "Request.hpp"
#include "Response.hpp"

#include <queue>

class Server;

class Connection {
	public:
		Connection();
		Connection(int fd, const Server *server);

		void receiveFromClient(short& events);
		void sendToClient(short& events);

	private:
		int					 m_fd;
		const Server		 *m_server;
		Request				 m_request;
		std::queue<Response> m_responses;
};
