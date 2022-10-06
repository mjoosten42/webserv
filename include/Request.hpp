#pragma once

#include "Server.hpp"

#include <map>
#include <string>

enum methods { NONE, GET, POST, DELETE };

class Request {
	public:
		Request(int fd, const Server *server, const std::string& total);

	private:
		int								   m_fd;
		methods							   m_method;
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
		const Server					  *m_server;
		std::string						   m_location;
};
