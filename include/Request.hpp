#pragma once

#include "HTTP.hpp"

#include <iostream>
#include <string>

enum methods { GET, POST, DELETE, NONE };

class Request: public HTTP {
	public:
		Request();

		void add(const char *str);
		void reset();
		void stringToData();

		const std::string& getLocation() const;
		methods			   getMethod() const;
		std::string		   getMethodAsString() const;

		void printMethod() const; //  TODO: remove

	private:
		void		parseStartLine();
		methods		parseMethod(const std::string	 &str) const;
		void		parseHeaders();
		std::string getNextLine();
		std::size_t newLineLength(std::size_t pos);

	private:
		std::string m_location;
		methods		m_method;
};

std::ostream& operator<<(std::ostream& os, const Request& request);
