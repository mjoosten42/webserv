#pragma once

#include "Server.hpp"

#include <map>
#include <string>

#define CRLF "\r\n"

class HTTP {
	public:
		HTTP(int fd, const Server *server);

	protected:
		int			  m_fd;
		const Server *m_server;

		std::string m_total;
		std::size_t m_pos;

		std::map<std::string, std::string> m_headers;

		std::string m_body;
};
