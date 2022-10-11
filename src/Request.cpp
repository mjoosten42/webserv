#include "Request.hpp"

#include "HTTP.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <string>

const static char *methodStrings[]	 = { "GET", "POST", "DELETE", "NONE" };
const static int   methodStringsSize = sizeof(methodStrings) / sizeof(*methodStrings);

Request::Request() {}

void Request::add(const char *str) {
	m_total += str;
}

void Request::stringToData() {
	parseStartLine();
	parseHeaders();

	m_total.erase(0, m_pos); //  Cut of start-line and headers, leaving only the body
	m_total.swap(m_body);	 //  No copying needed
}

std::string Request::getNextLine() {
	std::size_t pos = m_total.find_first_of(CRLF, m_pos); //  find index of next newline
	if (pos == std::string::npos)						  //  EOF
		return "";
	std::string line = m_total.substr(m_pos, pos - m_pos); //  extract the substring between old newline and new newline
	m_pos			 = pos + newLineLength(pos);		   //  update old newline, skipping \n or \r\n
	return line;
}

std::size_t Request::newLineLength(std::size_t pos) {
	if (m_total[pos] == '\n')
		return 1;
	if (m_total[pos + 1] == '\n') //  m_total[pos] must be \r
		return 2;
	std::cerr << "Newline error at " << pos << ": " << m_total << std::endl; //  TODO: assert
	return -1;
}

void Request::parseHeaders() {
	std::pair<std::map<std::string, std::string>::iterator, bool> insert;
	std::pair<std::string, std::string>							  header;
	std::istringstream											  line(getNextLine());

	line >> header.first;
	line >> header.second;
	while ((header.first != CRLF || header.first != "\n") && !header.first.empty()) {
		strToUpper(header.first);
		if (header.first.back() != ':')
			std::cerr << "Header field must end in ':' : " << header.first << std::endl;
		header.first.pop_back();
		insert = m_headers.insert(header);
		if (!insert.second)
			std::cerr << "Duplicate headers: " << header.first << std::endl;
		line.clear();
		line.str(getNextLine());
		header.first.clear();
		line >> header.first;
		line >> header.second;
		if (header.first.empty())
			break;
	}
}

const std::string& Request::getLocation() const {
	return m_location;
}

methods Request::getMethod() const {
	return m_method;
}

std::string Request::getMethodAsString() const {
	return methodStrings[m_method];
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
	//  TODO: send bad request

	if (m_location == "/")
		m_location += "html/";

	//  M: this depends on server
	//   serve index.html when the location ends with a /
	if (m_location.back() == '/')
		m_location += "index.html"; //  TODO: when index php, do just that instead etc.
}

methods Request::parseMethod(const std::string& str) const {
	for (int i = 0; i < methodStringsSize; i++)
		if (str == methodStrings[i])
			return static_cast<methods>(i);
	return NONE;
}

std::ostream& operator<<(std::ostream& os, const Request& request) {
	os << RED << "Location: " << DEFAULT << request.getLocation() << std::endl;
	os << RED << "Method: " << DEFAULT << request.getMethodAsString() << std::endl;
	os << RED << "Headers: {\n" << DEFAULT << getStringMapAsString(request.getHeaders()) << RED << "}\n";
	os << RED << "Body: " << DEFAULT << request.getBody();
	return os;
}
