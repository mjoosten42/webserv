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

	private:
		void handleGet();
		int	 handleGetWithStaticFile(const std::string &filename);
		int	 transferFile(std::ifstream &infile);
		int	 sendChunked(std::ifstream &infile);
		int	 sendSingle(std::ifstream &infile);
		void sendResponse() const;
		void sendFail(int code, const std::string& msg);

	private:
		shared_fd	  m_fd;
		const Server *m_server;
};
