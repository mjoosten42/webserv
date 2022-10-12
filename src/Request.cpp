#include "Request.hpp"

#include "HTTP.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <string>

const static char *methodStrings[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH" };
const static int   methodStringsSize = sizeof(methodStrings) / sizeof(*methodStrings);

bool	isHttpVersion(const std::string	  &str);
bool	isSupportedMethod(methods method);
methods parseMethod(const std::string& str);

Request::Request(): m_pos(0) {}

void Request::add(const char *str) {
	m_total += str;
}

int Request::ProcessRequest() {
	int status;

	if ((status = parseStartLine()) != 200)
		return status;
	if ((status = parseHeaders()) != 200)
		return status;

	m_total.erase(0, m_pos); //  Cut off start-line and headers, leaving only the body
	m_total.swap(m_body);	 //  No copying needed

	return 200;
}

int Request::parseStartLine() {
	std::istringstream line(getNextLine());
	std::string		   word;

	line >> word;
	m_method = parseMethod(word);
	if (m_method == static_cast<methods>(-1))
		return 400;
	if (!isSupportedMethod(m_method))
		return 501;

	line >> m_location;
	if (m_location.empty()) {
		std::cerr << "Empty location: { " << line.str() << " }\n";
		return 400;
	}

	word.clear();
	line >> word;
	if (!isHttpVersion(word)) {
		std::cerr << "Invalid HTTP version: { " << line.str() << " }\n";
		return 400;
	}
	if (word != "HTTP/1.1") {
		std::cerr << "HTTP 1.1 only: { " << line.str() << " }\n";
		return 505;
	}

	if (m_location == "/")
		m_location += "html/";

	//  M: this depends on server
	//   serve index.html when the location ends with a /
	if (m_location.back() == '/')
		m_location += "index.html"; //  TODO: when index php, do just that instead etc.

	return 200;
}

methods parseMethod(const std::string& str) {
	for (int i = 0; i < methodStringsSize; i++)
		if (str == methodStrings[i])
			return static_cast<methods>(i);
	std::cerr << "Incorrect method: " << str << std::endl;
	return static_cast<methods>(-1);
}

int Request::parseHeaders() {
	std::pair<std::map<std::string, std::string>::iterator, bool> insert;
	std::pair<std::string, std::string>							  header;
	std::istringstream											  line(getNextLine());

	line >> header.first;
	line >> header.second;
	while ((header.first != CRLF || header.first != "\n") && !header.first.empty()) {
		strToUpper(header.first);
		if (header.first.back() != ':') {
			std::cerr << "Header field must end in ':' : " << header.first << std::endl;
			return 400;
		}
		header.first.pop_back();
		insert = m_headers.insert(header);
		if (!insert.second) {
			std::cerr << "Duplicate headers: " << header.first << std::endl;
			return 400;
		}
		line.clear();
		line.str(getNextLine());
		header.first.clear();
		line >> header.first;
		line >> header.second;
	}

	return 200;
}

std::string Request::getNextLine() {
	std::size_t pos = m_total.find_first_of(CRLF, m_pos); //  find index of next newline
	if (pos == std::string::npos)						  //  EOF
		return "";
	std::string line = m_total.substr(m_pos, pos - m_pos); //  extract the substring between old newline and new newline
	m_pos			 = pos + newLineLength(pos);		   //  update old newline, skipping \n or \r\n
	return line;
}

std::size_t Request::newLineLength(std::size_t pos) const {
	if (m_total[pos] == '\n')
		return 1;
	if (m_total[pos + 1] == '\n') //  m_total[pos] must be \r
		return 2;
	std::cerr << "Newline error at " << pos << ": " << m_total << std::endl; //  TODO: assert
	return -1;
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
	m_location.clear();
	m_pos = 0;
}

std::ostream& operator<<(std::ostream& os, const Request& request) {
	os << RED << "Location: " << DEFAULT << request.getLocation() << std::endl;
	os << RED << "Method: " << DEFAULT << request.getMethodAsString() << std::endl;
	os << RED << "Headers: {\n" << DEFAULT << getStringMapAsString(request.getHeaders()) << RED << "}\n";
	os << RED << "Body: " << DEFAULT << request.getBody();
	return os;
}

bool isHttpVersion(const std::string& str) {
	return str.substr(0, 5) == "HTTP/" && std::isdigit(str[5]) && str[6] == '.' && std::isdigit(str[7]);
}

bool isSupportedMethod(methods method) {
	return method == GET || method == POST || method == DELETE;
}
