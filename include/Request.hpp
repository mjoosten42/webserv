#pragma once

#include "Server.hpp"

#include <map>
#include <string>

enum methods { GET, POST, DELETE };

enum progress { NONE, METHOD, LOCATION, HTTP, HEADERS, BODY, DONE };

class Request {
	public:
		Request(int fd, const Server *server);

		void addToRequest(const std::string& str);

	private:
		int								   m_fd;
		methods							   m_method;
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
		const Server					  *m_server;
		std::string						   m_location;
		progress						   m_progress;
};
