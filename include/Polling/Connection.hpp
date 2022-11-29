#pragma once

#include "FD.hpp"
#include "Request.hpp"
#include "Response.hpp"

#include <netinet/in.h>
#include <queue>

class Listener;
class Poller;

class Connection {
	public:
		Connection();
		Connection(FD fd, const Listener *listener, Poller *poller, const std::string &peer);

		short receive();
		short send();

		bool wantsClose() const;

	private:
		Response &getLastResponse();

	private:
		FD m_fd;

		const Listener *m_listener;
		Poller		   *m_poller;

		std::queue<Response> m_responses;
		std::string			 m_peer;
		bool				 m_close;
};
