#pragma once

#include "HTTP.hpp"

#include <string>

enum methods { GET, POST, DELETE, NONE };

class Request: public HTTP {
	public:
		Request();

		void add(const char *str);
		void reset();

		std::string& getLocation();
		methods		 getMethod() const;

	private:
		void	parseStartLine();
		methods parseMethod(const std::string& str) const;
		void	printMethod() const; //  TODO: remove

	private:
		std::string m_location;
		methods		m_method;
};
