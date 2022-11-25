#include "HTTP.hpp"

#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"

#include <string>
#include <utility>

// this function returns a pretty header field. Example: "host" becomes "Host",
// "transfer-encoding" becomes "Transfer-Encoding". Segfaults if field is empty.
std::string HTTP::capitalizeFieldPretty(std::string field) {
	size_t pos = 0;

	while (field.size() > pos) {
		field[pos] = std::toupper(field[pos]);
		pos		   = field.find_first_of('-', pos);
		if (pos == std::string::npos)
			break;
		pos++;
	}
	return field;
}

bool HTTP::containsNewline(const std::string &str) {
	return str.find(CRLF) != str.npos || str.find("\n") != str.npos;
}

size_t HTTP::findNewline(const std::string &str, size_t begin) {
	size_t pos = str.find(CRLF, begin);

	if (pos != std::string::npos)
		return pos;
	return str.find("\n", begin);
}

// Assumes ContainsNewline is called beforehand
// Automatically erases line from saved data
std::string HTTP::getNextLine() {
	std::size_t pos			  = findNewline(m_saved);
	std::string line		  = m_saved.substr(0, pos);
	int			newlineLength = (m_saved[pos] == '\r') ? 2 : 1; // "\r\n or \n"

	m_saved.erase(0, pos + newlineLength);
	return line;
}

bool HTTP::isGood() {
	return m_status < 400;
}

// parses a HTTP header and puts it into m_headers. NOTE: multiline headers are not supported.
void HTTP::parseHeader(const std::string &line) {
	std::pair<std::string, std::string> header;
	size_t								colonPos = line.find_first_of(':');

	if (colonPos == std::string::npos)
		throw ServerException(400, "Header field must end in ':' : " + line);

	// colonPos can never be string::npos from here
	header.first = line.substr(0, colonPos);
	if (!isHTTPToken(header.first))
		throw ServerException(400, "Header field is an invalid HTTP token: " + header.first);

	strToLower(header.first); // HTTP/1.1 headers are case-insensitive, so lowercase them.

	size_t valueStartPos = line.find_first_not_of(SPACE_AND_TAB, colonPos + 1);
	if (valueStartPos != std::string::npos) {
		header.second = line.substr(valueStartPos);
		header.second.erase(header.second.find_last_not_of(SPACE_AND_TAB) + 1, std::string::npos);
	}

	auto insert = m_headers.insert(header);
	if (!insert.second)
		throw ServerException(400, "Duplicate headers: " + line);
}

void HTTP::addToBody(const std::string &str) {
	m_body += str;
}

void HTTP::addToBody(const char *buf, ssize_t size) {
	m_body.append(buf, size);
}

void HTTP::addHeader(const std::string &field, const std::string &value) {
	std::string copy(field);

	if (hasHeader(field)) {
		LOG_ERR("Overwriting Header: " << field);
		LOG_ERR("Current headers: {\n" << getHeadersAsString() << "}");
	}

	strToLower(copy);
	m_headers[copy] = value;
}

bool HTTP::hasHeader(const std::string &field) const {
	std::string copy(field);

	strToLower(copy);
	return m_headers.find(copy) != m_headers.end();
}

void HTTP::removeHeader(const std::string &field) {
	std::string copy(field);

	strToLower(copy);
	m_headers.erase(copy);
}

int HTTP::getStatus() const {
	return m_status;
}

std::string &HTTP::getBody() {
	return m_body;
}

const std::string &HTTP::getBody() const {
	return m_body;
}

std::string HTTP::getHeader(const std::string &field) const {
	std::string copy(field);

	strToLower(copy);
	auto it = m_headers.find(copy);
	if (it != m_headers.end())
		return it->second;
	return "";
}

std::string HTTP::getHeadersAsString(const std::string &tabs) const {
	std::string headers;

	for (auto header : m_headers)
		headers += tabs + capitalizeFieldPretty(header.first) + ": " + header.second + CRLF;
	return headers;
}
