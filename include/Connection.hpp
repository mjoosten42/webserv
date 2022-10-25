#pragma once

#include "Request.hpp"
#include "Response.hpp"

#include <queue>

class Listener;

class Connection {
	public:
		Connection();
		Connection(int fd, const Listener *listener);

		void receiveFromClient(short& events);
		bool sendToClient(short& events);

	private:
		Response& getLastResponse();

	private:
		int					 m_fd;
		const Listener		*m_listener;
		std::queue<Response> m_responses;
};
