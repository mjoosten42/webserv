#pragma once

#include "HTTP.hpp"

#include <iostream>
#include <string>

enum methods { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

enum state { STARTLINE, HEADERS, BODY, DONE };

class Request: public HTTP {
	public:
		Request();

		void append(const char *buf, ssize_t size);

		const std::string& getHost() const;
		const state		 & getState() const;
		const methods	 & getMethod() const;
		const std::string& getLocation() const;
		const std::string& getQueryString() const;
		size_t			   getContentLength() const;
		size_t			   getBodyTotal() const;
		const std::string& getErrorMsg() const;

		std::string getStateAsString() const;
		std::string getMethodAsString() const;

	private:
		void parse();
		void checkSpecialHeaders();

		void parseStartLine(const std::string& line);
		void parseHeader(const std::string& line);

		void parseMethod(const std::string& str);
		void parseLocation(const std::string& str);
		void parseHTTPVersion(const std::string& str);

		std::string getNextLine();

	private:
		state		m_state;
		methods		m_method;	   // GET, POST, DELETE
		std::string m_location;	   // ex. /foo/bar.html
		std::string m_queryString; // ex. amongus=sus&greeting=Good%20morning

		std::string m_saved;
		std::string m_host;
		size_t		m_contentLength;
		size_t		m_bodyTotal;
		bool		m_processed;

		std::string m_errorMsg;
};

std::ostream& operator<<(std::ostream& os, const Request& request);
