#pragma once

#include "Server.hpp"

#include <map>
#include <string>

enum methods { GET, POST, DELETE };

enum state { READING, DONE };

class Request {
	public:
		Request(int fd, const std::string& total);

	private:
		int								   m_fd;
		methods							   m_method;
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
		state							   m_state;
		Server							  *m_server;
};
