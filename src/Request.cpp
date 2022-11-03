#include "Request.hpp"

#include "HTTP.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <sstream>
#include <string>

const static char *methodStrings[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH" };
const static int   methodStringsSize = sizeof(methodStrings) / sizeof(*methodStrings);

bool isHttpVersion(const std::string& str);
bool isSupportedMethod(methods method);
bool containsNewline(const std::string& str);

Request::Request():
	m_state(STARTLINE), m_method(static_cast<methods>(-1)), m_contentLength(0), m_bodyTotal(0), m_processed(false) {}

void Request::append(const char *buf, ssize_t size) {
	m_saved.append(buf, size);

	try {
		parse();
	} catch (int status) {
		m_state = DONE;
		LOG_ERR(m_errorMsg);
		throw;
	}
}

void Request::cut(ssize_t len) {
	m_body.erase(0, len);
}

void Request::clear() {
	HTTP::clear();
	m_state = STARTLINE;
	m_location.clear();
	m_queryString.clear();
	m_saved.clear();
	m_host.clear();
	m_bodyTotal		= 0;
	m_contentLength = 0;
}

void Request::parse() {
	std::string line;

	switch (m_state) {
		case STARTLINE:
			if (containsNewline(m_saved)) {
				line = getNextLine();
				if (line.empty()) {
					m_errorMsg = "Missing startline";
					throw 400;
				}
				parseStartLine(line);
				m_state = HEADERS;
				parse();
			}
			break;
		case HEADERS:
			while (containsNewline(m_saved)) {
				line = getNextLine();
				if (line.empty()) {
					m_state = BODY;
					parse();
					break;
				}
				parseHeader(line);
			}
			break;
		case BODY:
			m_bodyTotal += m_saved.length();
			addToBody(m_saved);
			m_saved.clear();
			if (!m_processed)
				checkSpecialHeaders();
			if (m_bodyTotal == m_contentLength)
				m_state = DONE;
			break;
		case DONE:
			LOG_ERR("Appending to DONE request"); // Shouldn't be reached
	}
}

// TODO: unit test this
void Request::parseStartLine(const std::string& line) {
	std::istringstream ss(line);
	std::string		   word;

	ss >> word;
	parseMethod(word);
	word.clear();

	ss >> word;
	parseLocation(word);
	word.clear();

	ss >> word;
	parseHTTPVersion(word);
	word.clear();

	ss >> word;
	if (!word.empty()) {
		m_errorMsg = "Extra text after HTTP version: " + line;
		throw 400;
	}
}

void Request::parseMethod(const std::string& str) {
	for (int i = 0; i < methodStringsSize; i++)
		if (str == methodStrings[i])
			m_method = static_cast<methods>(i);
	if (m_method == static_cast<methods>(-1)) {
		m_errorMsg = "Incorrect method: " + str;
		throw 400;
	}
	if (!isSupportedMethod(m_method)) {
		m_errorMsg = "Unsupported method: " + str;
		throw 501;
	}
}

void Request::parseLocation(const std::string& str) {
	size_t pos = str.find('?');

	if (str.empty()) {
		m_errorMsg = "Missing location: " + str;
		throw 400;
	}

	m_location = str.substr(0, pos);
	if (pos != std::string::npos)
		m_queryString = str.substr(pos + 1);
}

void Request::parseHTTPVersion(const std::string& str) {
	if (!isHttpVersion(str)) {
		m_errorMsg = "Invalid HTTP version: " + str;
		throw 400;
	}
	if (str != HTTP_VERSION) {
		m_errorMsg = std::string(HTTP_VERSION) + " only: " + str;
		throw 505;
	}
}

void Request::parseHeader(const std::string& line) {
	std::pair<MapIter, bool>			insert;
	std::pair<std::string, std::string> header;
	std::istringstream					ss(line);

	ss >> header.first;
	ss >> header.second;
	if (header.first.back() != ':') {
		m_errorMsg = "Header field must end in ':' : \"" + line + "\"";
		throw 400;
	}
	strToLower(header.first); // HTTP/1.1 headers are case-insensitive, so tolower them.
	header.first.pop_back();
	insert = m_headers.insert(header);
	if (!insert.second) {
		m_errorMsg = "Duplicate headers: \"" + line + "\"";
		throw 400;
	}
}

void Request::checkSpecialHeaders() {
	if (hasHeader("Host")) {
		std::string hostHeader = getHeaderValue("Host");
		m_host				   = hostHeader.substr(0, hostHeader.find(':'));
		if (m_host.empty()) {
			m_errorMsg = "Empty host";
			throw 400;
		}
	} else {
		m_errorMsg = "Missing host";
		throw 400;
	}
	if (hasHeader("Content-Length"))
		m_contentLength = stringToIntegral<std::size_t>(getHeaderValue("Content-Length"));
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

// Assumes ContainsNewline is called beforehand
// Automatically erases line from saved data
std::string Request::getNextLine() {
	std::size_t pos			  = findNewline(m_saved);
	std::string line		  = m_saved.substr(0, pos);
	int			newlineLength = (m_saved[pos] == '\r') ? 2 : 1; // "\r\n or \n"

	m_saved.erase(0, pos + newlineLength);
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

const std::string& Request::getErrorMsg() const {
	return m_errorMsg;
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
		case DONE:
			return "DONE";
	}
}

size_t Request::getContentLength() const {
	return m_contentLength;
}

size_t Request::getBodyTotal() const {
	return m_bodyTotal;
}

std::ostream& operator<<(std::ostream& os, const Request& request) {
	os << RED "State: " DEFAULT << request.getStateAsString() << std::endl;
	os << RED "Method: " DEFAULT << request.getMethodAsString() << std::endl;
	os << RED "Location: " DEFAULT << request.getLocation() << std::endl;
	if (!request.getQueryString().empty())
		os << RED "Query string: " DEFAULT << request.getQueryString() << std::endl;
	// if (!request.getHeaders().empty())
	// 	os << RED "Headers: {\n" DEFAULT << getStringMapAsString(request.getHeaders()) << RED << "}\n";
	os << RED "Host: " DEFAULT << request.getHost() << std::endl;
	os << RED "Content-Length: " DEFAULT << request.getContentLength() << std::endl;
	if (!request.getBody().empty())
		os << RED "Body: " DEFAULT << request.getBody() << std::endl;
	os << RED "Body total: " DEFAULT << request.getBodyTotal();
	return os;
}
