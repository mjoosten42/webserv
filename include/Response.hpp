#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
	public:
		Response(int fd, const Server *server);
	
		bool		sendResponse(void) const;
		std::string statusLine() const;
		std::string getResponseAsCPPString(void) const;

	private:
		std::string statusMsg(int code) const;

	private:
		int m_statusCode;
};
