#pragma once

#include "Server.hpp"

#include <map>
#include <string>

enum methods { GET, POST, DELETE };

enum progress { METHOD, LOCATION, HTTP, HEADERS, BODY, DONE };

class Request {
	public:
		Request(int fd, const Server *server);

		void addToRequest(const std::string& str);

	private:
		int								   m_fd;
		const Server					  *m_server;
		progress						   m_progress;
		methods							   m_method;
		std::string						   m_location;
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
};
