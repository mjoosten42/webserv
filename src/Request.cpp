#include "Request.hpp"

#include "HTTP.hpp"
#include "utils.hpp"

#include <sstream>
#include <string>

Request::Request() {}

void Request::add(const char *str) {
	m_total += str;
}

std::string& Request::getLocation() {
	return m_location;
}

void Request::reset() {
	m_total = "";
	m_method = "";
	m_location = "";
}

void Request::parseStartLine() {
	std::istringstream line(getNextLine());
	std::string		   word;

	line >> word;
	if ((m_method = testMethod(word)) == "")
		std::cerr << "Incorrect method: " << word << std::endl;

	line >> m_location;
	if (m_location.empty())
		std::cerr << "Empty location: " << m_location << std::endl;

	word.clear();
	line >> word;
	if (word != "HTTP/1.1")
		std::cerr << "HTTP 1.1 only: " << word << std::endl;

	// serve index.html when the location ends with a /
	if (m_location.back() == '/')
	{
		m_location += "index.html"; // TODO: when index php, do just that instead etc.
	}

	std::cout << "Method: " << m_method << std::endl;
	std::cout << "Location: " << m_location << std::endl;
}
