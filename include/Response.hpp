#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
	public:
		Response();

		std::string		   getStatusLine() const;
		std::string		   getStatusMessage() const;
		std::string		   getResponseAsString(void);
		const std::string& getBody() const;

		void addToBody(const std::string& str);
		void addHeader(const std::string& field, const std::string& value);

		void reset();

	private:
		std::string getHeadersAsString() const;

	public:
		int m_statusCode;
};
