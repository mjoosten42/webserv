#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

class Handler {
	public:
		Handler();
		Handler(int fd, const Server *server);
		~Handler();

		void sendResponse() const;

	public:
		Request	 m_request;
		Response m_response;

	private:
		int			  m_fd;
		const Server *m_server;
};
