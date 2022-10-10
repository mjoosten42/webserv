#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
	public:
		Response();

		std::string statusLine() const;
		std::string statusMessage(int code) const;
		std::string getResponseAsCPPString(void) const;

		void setStatusCode(int code);

	private:
		void parseStartLine();

	private:
		int m_statusCode;
};
