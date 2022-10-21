#include "Request.hpp"

#include "HTTP.hpp"
#include "defines.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <string>

const static char *methodStrings[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH" };
const static int   methodStringsSize = sizeof(methodStrings) / sizeof(*methodStrings);

bool isHttpVersion(const std::string& str);
bool isSupportedMethod(methods method);
bool containsNewline(const std::string& str);

methods		parseMethod(const std::string	 &str);
std::size_t findNewline(const std::string& str);

Request::Request(): m_state(STARTLINE), m_contentLength(0) {}

// Body isn't being added
void Request::add(const char *str) {
	std::string line;
	m_saved += str;

	while (containsNewline(m_saved)) {
		switch (m_state) {
			case STARTLINE:
				if ((line = getNextLine()).empty()) {
					std::cerr << "Missing startline\n";
					return;
				}
				if (parseStartLine(line) != 200)
					return;
				m_state = HEADERS;
				break;
			case HEADERS:
				if ((line = getNextLine()).empty()) {
					if (!hasHeader("Host")) {
						std::cerr << "Missing host header\n";
						return;
					}
					m_state = BODY;
					if (m_method == GET || m_method == HEAD ||
						(hasHeader("Content-Length") &&
						 stringToIntegral<std::size_t>(getHeaderValue("Content-Length")) == m_body.length()))
						m_state = DONE;
					break;
				}
				if (parseHeader(line) != 200)
					return;
				break;
			case BODY:
				addToBody(m_saved);
				m_saved.clear();
				return;
			case DONE:
				std::cerr << "Adding to closed request\n";
		}
	}
}

void Request::cut(ssize_t len) {
	m_body.erase(0, len);
}

int Request::parseStartLine(const std::string& line) {
	std::istringstream ss(line);
	std::string		   word;

	ss >> word;
	m_method = parseMethod(word);
	if (m_method == static_cast<methods>(-1))
		return 400;
	if (!isSupportedMethod(m_method))
		return 501;

	ss >> m_location;
	if (m_location.empty()) {
		std::cerr << "Empty location: " << ss.str() << "\n";
		return 400;
	}
	//  parse query string and location
	size_t questionMarkPos = m_location.find('?');
	if (questionMarkPos != std::string::npos) {
		m_queryString = m_location.substr(questionMarkPos + 1);
		m_location.erase(m_location.begin() + questionMarkPos);
	}

	word.clear();
	ss >> word;
	if (!isHttpVersion(word)) {
		std::cerr << "Invalid HTTP version: " << ss.str() << "\n";
		return 400;
	}
	if (word != "HTTP/1.1") {
		std::cerr << "HTTP 1.1 only: " << ss.str() << "\n";
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

int Request::parseHeader(const std::string& line) {
	std::pair<std::map<std::string, std::string>::iterator, bool> insert;
	std::pair<std::string, std::string>							  header;
	std::istringstream											  ss(line);

	ss >> header.first;
	ss >> header.second;
	if (header.first.back() != ':') {
		std::cerr << RED "Header field must end in ':' : " << line << DEFAULT << std::endl;
		return 400;
	}
	header.first.pop_back();
	insert = m_headers.insert(header);
	if (!insert.second) {
		std::cerr << RED "Duplicate headers: " << line << DEFAULT << std::endl;
		return 400;
	}
	checkSpecialHeaders(header);
	return 200;
}

void Request::checkSpecialHeaders(const std::pair<std::string, std::string>& header) {
	if (header.first == "Content-Length")
		m_contentLength = stringToIntegral<std::size_t>(header.second);
	if (header.first == "Host")
		m_host = header.second.substr(0, header.second.find(':'));
}

//  Assumes ContainsNewline is called beforehand
//  Automatically erases line from saved data
std::string Request::getNextLine() {
	std::size_t pos			  = findNewline(m_saved);
	std::string line		  = m_saved.substr(0, pos);
	int			newlineLength = (m_saved[pos] == '\r') ? 2 : 1;

	m_saved.erase(m_saved.begin(), m_saved.begin() + pos + newlineLength);
	return line;
}

const std::string& Request::getLocation() const {
	return m_location;
}

const std::string& Request::getQueryString() const {
	return m_queryString;
}

const methods& Request::getMethod() const {
	return m_method;
}

const state& Request::getState() const {
	return m_state;
}

const std::string& Request::getHost() const {
	return m_host;
}

std::string Request::getMethodAsString() const {
	switch (m_method) {
		case GET:
			return "GET";
		case POST:
			return "POST";
		case DELETE:
			return "DELETE";
		default:
			return "NONE";
	}
}

std::string Request::getStateAsString() const {
	switch (m_state) {
		case STARTLINE:
			return "STARTLINE";
		case HEADERS:
			return "HEADERS";
		case BODY:
			return "BODY";
		default:
			return "DONE";
	}
}

std::ostream& operator<<(std::ostream& os, const Request& request) {
	os << RED "State: " DEFAULT << request.getStateAsString() << std::endl;
	os << RED "Method: " DEFAULT << request.getMethodAsString() << std::endl;
	os << RED "Location: " DEFAULT << request.getLocation() << std::endl;
	os << RED "Query string: " DEFAULT << request.getQueryString() << std::endl;
	os << RED "Headers: {\n" DEFAULT << getStringMapAsString(request.getHeaders()) << RED << "}\n";
	os << RED "Host: " DEFAULT << request.getHost() << std::endl;
	os << RED "Body: " DEFAULT << request.getBody();
	return os;
}

bool isHttpVersion(const std::string& str) {
	return str.substr(0, 5) == "HTTP/" && std::isdigit(str[5]) && str[6] == '.' && std::isdigit(str[7]);
}

bool isSupportedMethod(methods method) {
	return method == GET || method == POST || method == DELETE;
}

bool containsNewline(const std::string& str) {
	return str.find("\r\n") != str.npos || str.find("\n") != str.npos;
}

std::size_t findNewline(const std::string& str) {
	std::size_t pos = str.find("\r\n");
	if (pos != str.npos)
		return pos;
	return str.find("\n");
}
