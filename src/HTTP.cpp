#include "HTTP.hpp"

#include "defines.hpp"
#include "stringutils.hpp"

#include <string>
#include <utility>

HTTP::HTTP() {}

void HTTP::clear() {
	m_headers.clear();
	m_body.clear();
}

const std::string& HTTP::getBody() const {
	return m_body;
}

// this function returns a pretty header field. Example: "host" becomes "Host",
// "transfer-encoding" becomes "Transfer-Encoding". Segfaults if field is empty.
std::string HTTP::capitalizeFieldPretty(std::string field) {
	field[0] = std::toupper(field[0]);

	size_t pos = 0;

	while (true) {

		pos = field.find_first_of('-', pos);
		if (pos == std::string::npos) {
			break;
		}
		pos++;
		if (pos >= field.length()) {
			break;
		}
		field[pos] = std::toupper(field[pos]);
	}
	return field;
}

const std::map<std::string, std::string>& HTTP::getHeaders() const {
	return m_headers;
}

void HTTP::addToBody(const std::string& str) {
	m_body += str;
}

void HTTP::addToBody(const char *buf, ssize_t size) {
	m_body.append(buf, size);
}

// this function assumes the field does not contain any whitespace.
// TODO: should we just remove the strtolower for performance reasons and just make it
// mandatory to _always_ call it with lowercase?
void HTTP::addHeader(std::string field, const std::string& value) {
	strToLower(field);
	m_headers[field] = value;
}

// ALWAYS LOOK UP WITH LOWERCASE HEADER FIELDS
bool HTTP::hasHeader(const std::string& field) const {
	return m_headers.find(field) != m_headers.end();
}

// ALWAYS LOOK UP WITH LOWERCASE HEADER FIELDS
std::string HTTP::getHeaderValue(const std::string& field) {
	return m_headers.find(field)->second;
}

std::string HTTP::getHeadersAsString() const {
	std::map<std::string, std::string>::const_iterator it;
	std::string										   headers;

	for (it = m_headers.begin(); it != m_headers.end(); it++)
		headers += HTTP::capitalizeFieldPretty(it->first) + ": " + it->second + CRLF;
	return headers;
}
