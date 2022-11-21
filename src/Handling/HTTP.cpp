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

void HTTP::parseHeader(const std::string &line) {
	std::pair<std::string, std::string> header;
	size_t								pos = line.find_first_of(IFS);

	header.first = line.substr(0, pos);
	if (pos != std::string::npos)
		header.second = line.substr(line.find_first_not_of(IFS, pos));

	if (header.first.back() != ':')
		throw ServerException(400, "Header field must end in ':' : " + line);
	header.first.pop_back();
	strToLower(header.first); // HTTP/1.1 headers are case-insensitive, so lowercase them.
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

int HTTP::getStatus() const {
	return m_status;
}

std::string &HTTP::getBody() {
	return m_body;
}

const std::string &HTTP::getBody() const {
	return m_body;
}

std::string HTTP::getHeaderValue(const std::string &field) const {
	std::string copy(field);

	strToLower(copy);
	auto it = m_headers.find(copy);
	if (it != m_headers.end())
		return it->second;
	return "";
}

std::string HTTP::getHeadersAsString() const {
	std::string headers;

	for (auto header : m_headers)
		headers += capitalizeFieldPretty(header.first) + ": " + header.second + CRLF;
	return headers;
}
