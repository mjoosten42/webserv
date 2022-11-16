#pragma once

#include "FD.hpp"
#include "Request.hpp"
#include "Response.hpp"

#include <netinet/in.h>
#include <queue>

class Listener;

class Connection {
	public:
		Connection();
		Connection(FD fd, const Listener *listener, const std::string& peer);

		FD receive(short& events);
		FD send(short& events);

		bool wantsClose() const;

	private:
		Response& getLastResponse();

	private:
		FD m_fd;

		std::queue<Response> m_responses;
		const Listener		*m_listener;
		std::string			 m_peer;
		bool				 m_close;
};
