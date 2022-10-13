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
		int	 transferFile(std::ifstream &infile);
		int	 sendChunked(std::ifstream &infile);
		int	 sendSingle(std::ifstream &infile);
		int  handleCGI(const std::string& command, const std::string& filename);
		void sendResponse();

	private:
		shared_fd	  m_fd;
		const Server *m_server;
};
