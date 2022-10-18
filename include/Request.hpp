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
		int	 ProcessRequest();
		void clear();

		const std::string& getLocation() const;
		const std::string& getQueryString() const;
		methods			   getMethod() const;
		std::string		   getMethodAsString() const;
		state			   getState() const;
		std::string		   getStateAsString() const;

	private:
		int			parseStartLine(const std::string		&line);
		int			parseHeader(const std::string		 &line);
		std::string getNextLine();
		std::size_t newLineLength(std::size_t pos) const;

	private:
		std::string m_location;	   //  ex. /foo/bar.html
		std::string m_queryString; //  ex. amongus=sus&greeting=Good%20morning
		methods		m_method;	   //  GET, POST, etc.
		state		m_state;
		std::string m_saved;
		std::size_t m_contentLength;
};

std::ostream& operator<<(std::ostream& os, const Request& request);
