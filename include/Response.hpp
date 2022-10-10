#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
	public:
		Response();

		int			getStatusCode() const;
		std::string getStatusLine() const;
		std::string getStatusMessage(int code) const;
		std::string getResponseAsCPPString(void) const;

	private:
		void parseStartLine();

	private:
		int m_statusCode;
};
