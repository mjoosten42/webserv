#pragma once

#include "Request.hpp"
#include "Response.hpp"

#include <netinet/in.h>
#include <queue>

class Listener;

class Connection {
	public:
		Connection();
		Connection(int fd, const Listener *listener, const std::string& peer);

		int receiveFromClient(short& events);
		int sendToClient(short& events);

		bool wantsClose() const;

	private:
		Response& getLastResponse();

	private:
		int m_fd;

		std::queue<Response> m_responses;
		const Listener		*m_listener;
		std::string			 m_peer;
		bool				 m_close;
};
