#include "Request.hpp"

#include "HTTP.hpp"
#include "utils.hpp"

#include <sstream>
#include <string>

const static char *methodStrings[]	 = { "GET", "POST", "DELETE", "NONE" };
const static int   methodStringsSize = sizeof(methodStrings) / sizeof(*methodStrings);

Request::Request() {}

void Request::add(const char *str) {
	m_total += str;
}

std::string& Request::getLocation() {
	return m_location;
}

methods Request::getMethod() const {
	return m_method;
}

//  TODO: remove
void Request::printMethod() const {
	std::cout << "Method: " << methodStrings[m_method] << std::endl;
}

void Request::reset() {
	HTTP::reset();
	m_total.clear();
	m_method = NONE;
	m_location.clear();
}

void Request::parseStartLine() {
	std::istringstream line(getNextLine());
	std::string		   word;

	line >> word;
	m_method = parseMethod(word);
	if (m_method == NONE)
		std::cerr << "Incorrect method: " << word << std::endl;

	line >> m_location;
	if (m_location.empty())
		std::cerr << "Empty location: " << m_location << std::endl;

	word.clear();
	line >> word;
	if (word != "HTTP/1.1")
		std::cerr << "HTTP 1.1 only: " << word << std::endl;

	if (m_location == "/")
		m_location += "html/";

	//  M: this depends on server
	//   serve index.html when the location ends with a /
	if (m_location.back() == '/')
		m_location += "index.html"; //  TODO: when index php, do just that instead etc.

	printMethod();
	std::cout << "Location: " << m_location << std::endl;
}

methods Request::parseMethod(const std::string& str) const {
	for (int i = 0; i < methodStringsSize; i++)
		if (str == methodStrings[i])
			return static_cast<methods>(i);
	return NONE;
}
