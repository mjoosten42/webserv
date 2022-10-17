#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

#include <string>

class Handler {
	public:
		Handler(Request& request, Response& response, const Server *server);

		void clear();
		void handle();

	private:
		Request & m_request;
		Response& m_response;

		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& location);
		void handleGet();
		int	 handleCGI(const std::string &command, const std::string &filename);
		int	 handleGetWithStaticFile(const std::string &filename);
		int	 transferFile(int fd);
		int	 sendChunked(int fd);
		int	 sendSingle(int fd);

	private:
		const Server *m_server;
};
