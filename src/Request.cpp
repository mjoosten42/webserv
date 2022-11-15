#include "Request.hpp"

#include "HTTP.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <algorithm> // std::min
#include <string>

const static char *methodStrings[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH" };
const static int   methodStringsSize = sizeof(methodStrings) / sizeof(*methodStrings);

bool isHttpVersion(const std::string& str);
bool isSupportedMethod(methods method);

Request::Request(): m_state(STARTLINE), m_method(static_cast<methods>(-1)), m_contentLength(0), m_bodyTotal(0) {}

void Request::append(const char *buf, ssize_t size) {
	m_saved.append(buf, size);

	try {
		parse();
		m_status = 200;
	} catch (const ServerException& e) {
		m_state	   = DONE;
		m_status   = e.code;
		m_errorMsg = e.what();
	}
}

void Request::parse() {
	std::string line;

	switch (m_state) {
		case STARTLINE:
			if (containsNewline(m_saved)) {
				line = getNextLine();
				if (line.empty())
					throw ServerException(400, "Missing startline");
				parseStartLine(line);
				m_state = HEADERS;
				parse();
			}
			break;
		case HEADERS:
			while (containsNewline(m_saved)) {
				line = getNextLine();
				if (line.empty()) {
					checkSpecialHeaders();
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
			if (m_bodyTotal == m_contentLength)
				m_state = DONE;
			break;
		case DONE:
			LOG_ERR("Appending to DONE request"); // Shouldn't be reached
	}
}

// TODO: unit test this
void Request::parseStartLine(const std::string& line) {
	const char				*errorMessages[] = { "Missing startline", "Missing location", "Missing HTTP version" };
	std::vector<std::string> strs			 = stringSplit(line);

	if (strs.size() != 3) {
		if (strs.size() < 3)
			throw ServerException(400, errorMessages[strs.size()]);
		else
			throw ServerException(400, "Extra info after HTTP version");
	}

	parseMethod(strs[0]);
	parseURI(strs[1]);
	parseHTTPVersion(strs[2]);
}

void Request::parseMethod(const std::string& str) {
	for (int i = 0; i < methodStringsSize; i++)
		if (str == methodStrings[i])
			m_method = static_cast<methods>(i);
	if (m_method == static_cast<methods>(-1))
		throw ServerException(400, "Incorrect method: " + str);
	if (!isSupportedMethod(m_method))
		throw ServerException(501, "Unsupported method: " + str);
}

void Request::parseURI(const std::string& str) {
	if (str.find("..") != std::string::npos)
		throw ServerException(400, "Location contains \"..\": " + str);

	size_t dot	 = str.find('.');
	size_t extra = str.find_first_of("/?", dot);

	m_location = str.substr(0, extra);

	if (extra != std::string::npos) {
		size_t qm	 = str.find('?', extra);
	
		if (str[extra] == '/') {
			m_pathInfo = str.substr(extra, qm - extra);

			if (qm != std::string::npos && str.size() > qm)
				m_queryString = str.substr(qm + 1);
		}

		if (str[extra] == '?' && str.size() > qm)
			m_queryString = str.substr(qm + 1);
	}
}

void Request::parseHTTPVersion(const std::string& str) {
	if (!isHttpVersion(str))
		throw ServerException(400, "Invalid HTTP version: " + str);
	if (str != HTTP_VERSION)
		throw ServerException(505, HTTP_VERSION " only: " + str);
}

void Request::checkSpecialHeaders() {
	if (hasHeader("Host")) {
		std::string hostHeader = getHeaderValue("Host");
		m_host				   = hostHeader.substr(0, hostHeader.find(':'));
		if (m_host.empty())
			throw ServerException(400, "Empty host header");
	} else
		throw ServerException(400, "Missing host header");
	if (hasHeader("Content-Length"))
		m_contentLength = stringToIntegral<std::size_t>(getHeaderValue("Content-Length"));
}

bool isHttpVersion(const std::string& str) {
	return !str.compare(0, 5, "HTTP/") && std::isdigit(str[5]) && str[6] == '.' && std::isdigit(str[7]);
}

bool isSupportedMethod(methods method) {
	return method == GET || method == POST || method == DELETE;
}

#pragma region accessors

const std::string& Request::getLocation() const {
	return m_location;
}

const std::string& Request::getPathInfo() const {
	return m_pathInfo;
}

const std::string& Request::getQueryString() const {
	return m_queryString;
}

const std::string& Request::getHost() const {
	return m_host;
}

const std::string& Request::getErrorMsg() const {
	return m_errorMsg;
}

methods Request::getMethod() const {
	return m_method;
}

state Request::getState() const {
	return m_state;
}

size_t Request::getContentLength() const {
	return m_contentLength;
}

size_t Request::getBodyTotal() const {
	return m_bodyTotal;
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
			LOG_ERR("Invalid method: " << m_state);
			return "INVALID METHOD";
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
		default:
			LOG_ERR("Invalid state: " << m_state);
			return "INVALID STATE";
	}
}

#pragma endregion

std::ostream& operator<<(std::ostream& os, const Request& request) {
	os << RED "State: " DEFAULT << request.getStateAsString() << std::endl;
	os << RED "Method: " DEFAULT << request.getMethodAsString() << std::endl;
	os << RED "Location: " DEFAULT << request.getLocation() << std::endl;
	os << RED "Query string: " DEFAULT << request.getQueryString() << std::endl;
	os << RED "Path info: " DEFAULT << request.getPathInfo() << std::endl;
	// os << RED "Headers: {\n" DEFAULT << request.getHeadersAsString() << RED << "}\n";
	os << RED "Host: " DEFAULT << request.getHost() << std::endl;
	os << RED "Content-Length: " DEFAULT << request.getContentLength() << std::endl;
	// os << RED "Body: " DEFAULT << request.getBody() << std::endl;
	os << RED "Body total: " DEFAULT << request.getBodyTotal() << std::endl;
	// os << RED "Status: " DEFAULT << request.getStatus();
	return os;
}
