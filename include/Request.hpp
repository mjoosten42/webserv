#pragma once

#include "HTTP.hpp"

#include <string>

class Request: public HTTP {
	public:
		Request();

		void		 add(const char *str);
		std::string& getLocation();

	private:
		void parseStartLine();

	private:
		std::string m_location;
		std::string m_method;
};
