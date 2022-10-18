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
		void sendToClient(short& events);

	private:
		int					 m_fd;
		const Listener	   *m_listener;
		Request				 m_request;
		std::queue<Response> m_responses;
};
