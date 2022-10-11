#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "shared_fd.hpp"

enum state { EMPTY, READING, PROCESSING, WRITING, DONE };

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
		void sendFail(int code, const std::string& msg);
		void sendResponse() const;
		int	 transferFile(std::ifstream &infile);
		int	 sendSingle(std::ifstream &infile);
		int	 sendChunked(std::ifstream &infile);

	private:
		shared_fd	  m_fd;
		const Server *m_server;
};
