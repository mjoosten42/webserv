#pragma once

#include "HTTP.hpp"

#include <iostream>
#include <string>

enum methods { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

class Request: public HTTP {
	public:
		Request();

		void add(const char *str);
		void reset();
		int	 ProcessRequest();

		const std::string& getLocation() const;
		const std::string& getQueryString() const;
		methods			   getMethod() const;
		std::string		   getMethodAsString() const;

		void printMethod() const; //  TODO: remove

	private:
		int			parseStartLine();
		int			parseHeaders();
		std::string getNextLine();
		std::size_t newLineLength(std::size_t pos) const;

	private:
		std::string m_location;	   //  ex. /foo/bar.html
		std::string m_queryString; //  ex. amongus=sus&greeting=Good%20morning
		methods		m_method;	   //  GET, POST, etc.
		std::string m_total;
		std::size_t m_pos;
};

std::ostream& operator<<(std::ostream& os, const Request& request);
