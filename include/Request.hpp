#pragma once

#include "HTTP.hpp"

#include <iostream>
#include <string>

enum methods { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

class Request: public HTTP {
	public:
		Request();

		void add(const char *str);
		int	 ProcessRequest();
		void clear();

		const std::string& getLocation() const;
		methods			   getMethod() const;
		std::string		   getMethodAsString() const;

	private:
		int			parseStartLine();
		int			parseHeaders();
		std::string getNextLine();
		std::size_t newLineLength(std::size_t pos) const;

	private:
		std::string m_location;
		methods		m_method;
		std::string m_total;
		std::size_t m_pos;

		enum state { READING, PROCESSING, DONE } m_state;
};

std::ostream& operator<<(std::ostream& os, const Request& request);
