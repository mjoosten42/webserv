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

		const state	 & getState() const;
		const methods& getMethod() const;

		const std::string& getHost() const;
		const std::string& getLocation() const;
		void			   setLocation(const std::string);
		const std::string& getQueryString() const;
		const std::string& getPathInfo() const;
		const std::string& getErrorMsg() const;

		size_t getContentLength() const;
		size_t getBodyTotal() const;
		int	   getStatus() const;

		std::string getStateAsString() const;
		std::string getMethodAsString() const;

	private:
		void parse();
		void checkSpecialHeaders();

		void parseStartLine(const std::string& line);
		void parseMethod(const std::string& str);
		void parseLocation(const std::string& str);
		void parseHTTPVersion(const std::string& str);

	private:
		state		m_state;
		methods		m_method;	   // GET, POST, DELETE
		std::string m_location;	   // ex. /foo/bar.html
		std::string m_queryString; // ex. amongus=sus&greeting=Good%20morning
		std::string m_pathInfo;	   // /info.php/another/path

		std::string m_host;
		size_t		m_contentLength;
		size_t		m_bodyTotal;
		bool		m_processedHeaders;
		int			m_status;

		std::string m_errorMsg;
};

std::ostream& operator<<(std::ostream& os, const Request& request);
