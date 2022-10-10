#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
	public:
		Response();

		std::string statusLine() const;
		std::string getResponseAsCPPString(void) const;

		static std::string statusMsg(int code); //  TODO: rename

	private:
		int m_statusCode;
};
