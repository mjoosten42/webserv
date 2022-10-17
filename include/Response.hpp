#pragma once

#include "HTTP.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
	public:
		Response();

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsString(void) const;

		void clear();

	private:
		std::string getHeadersAsString() const;

	public:
		int m_statusCode;

		enum state { PROCESSING, WRITING, DONE } m_state;
};
