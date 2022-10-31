#pragma once

#include "Request.hpp"
#include "Response.hpp"

#include <queue>

class Listener;

class Connection {
	public:
		Connection();
		Connection(int fd, const Listener *listener);

		int receiveFromClient(short& events);
		int sendToClient(short& events);

		bool wantsClose() const;

	private:
		Response& getLastResponse();

	private:
		int m_fd;

		std::queue<Response> m_responses;
		const Listener		*m_listener;
		bool				 m_close;
};
