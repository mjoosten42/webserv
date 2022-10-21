#pragma once

#include "HTTP.hpp"

#include <iostream>
#include <string>

enum methods { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

enum state { STARTLINE, HEADERS, BODY, DONE };

class Request: public HTTP {
	public:
		Request();

		void add(const char *str);
		void cut(ssize_t len);

		const std::string& getHost() const;
		const state		 & getState() const;
		const methods	 & getMethod() const;
		const std::string& getLocation() const;
		const std::string& getQueryString() const;
		int				   getContentLength() const;

		std::string getStateAsString() const;
		std::string getMethodAsString() const;

	private:
		void clear();
		int	 parse();
		int	 parseStartLine(const std::string &line);
		int	 parseHeader(const std::string &line);
		int	 checkSpecialHeaders();

		std::string getNextLine();

	private:
		std::string m_location;	   //  ex. /foo/bar.html
		std::string m_queryString; //  ex. amongus=sus&greeting=Good%20morning
		methods		m_method;	   //  GET, POST, etc.
		state		m_state;
		std::string m_saved;
		std::size_t m_contentLength;
		std::string m_host;
};

std::ostream& operator<<(std::ostream& os, const Request& request);
