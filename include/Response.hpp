#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
	public:
		Response();

		int			getStatusCode() const;
		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsCPPString(void) const;

		void setStatusCode(int code);
		void addToBody(const std::string& str);
		void addHeader(const std::string& field, const std::string& value);

		void reset();

	private:
		void parseStartLine();

	private:
		int m_statusCode;
};
