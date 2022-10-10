#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "shared_fd.hpp"

class Handler {
	public:
		Handler();
		Handler(int fd, const Server *server);

		void sendResponse() const;

	public:
		Request	 m_request;
		Response m_response;

	private:
		shared_fd	  m_fd;
		const Server *m_server;
};
