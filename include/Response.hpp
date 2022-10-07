#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
	public:
		Response();
		Response(int fd, const Server *server);

		bool		sendResponse(void) const;
		std::string statusLine() const;
		std::string getResponseAsCPPString(void) const;

		static std::string statusMsg(int code); //  TODO: rename

	private:
		int m_statusCode;
};
