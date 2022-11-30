#pragma once

#include "FD.hpp"
#include "Request.hpp"
#include "Response.hpp"

#include <memory> // shared_ptr
#include <queue>

class Listener;
class Poller;

class Connection {
	public:
		Connection();
		Connection(FD fd, const Listener *listener, Poller *poller);

		short receive();
		short send();

		int getFD() const;
		int getFirstReadFD() const;

	private:
		Response &getLastResponse();

	private:
		FD m_fd;

		const Listener *m_listener;
		Poller		   *m_poller;

		std::queue<std::shared_ptr<Response> > m_responses;
};
