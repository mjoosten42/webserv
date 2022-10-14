#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

#include <queue>

class Connection {
	public:
		Connection();
		Connection(int fd, const Server *server);

		void receiveFromClient(short& events);
		void sendToClient(short& events);

	private:
		int					 m_fd;
		const Server		*m_server;
		std::queue<Request>	 m_requests;
		std::queue<Response> m_responses;
};
