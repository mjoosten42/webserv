#pragma once

#include "HTTP.hpp"

#include <string>

class Request: public HTTP {
	public:
		Request();

		void		 add(const char *str);
		void		 stringToData();
		std::string& getLocation();

	private:
		std::string getNextLine();
		std::size_t newLineLength(std::size_t pos);
		std::string testMethod(const std::string& str) const;
		void		parseStartLine();
		void		parseHeaders();

	private:
		std::string m_location;
		std::string m_method;
		std::size_t m_pos;
};
