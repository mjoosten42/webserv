#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "shared_fd.hpp"

class Handler {
	public:
		Handler();
		Handler(int fd, const Server *server);

		void reset();
		void handle();

	public:
		Request	 m_request;
		Response m_response;

		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& location);

	private:
		void handleGet();
		int	 handleGetWithStaticFile(const std::string &filename);
		int	 transferFile(int readfd);
		int	 sendChunked(int readfd);
		int	 sendSingle(int readfd);
		void sendResponse();

	private:
		shared_fd	  m_fd;
		const Server *m_server;
};
