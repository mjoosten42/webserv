#include "HTTP.hpp"

#include "defines.hpp"
#include "logger.hpp"
#include "stringutils.hpp"

#include <string>
#include <utility>

HTTP::HTTP() {}

void HTTP::clear() {
	m_headers.clear();
	m_body.clear();
}

// this function returns a pretty header field. Example: "host" becomes "Host",
// "transfer-encoding" becomes "Transfer-Encoding". Segfaults if field is empty.
std::string HTTP::capitalizeFieldPretty(std::string field) {
	field[0] = std::toupper(field[0]);

	size_t pos = 0;

	while (true) {

		pos = field.find_first_of('-', pos);
		if (pos == std::string::npos)
			break;
		pos++;
		if (pos >= field.length())
			break;
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

void HTTP::addHeader(const std::string& field, const std::string& value) {
	std::string copy(field);

	if (hasHeader(field)) {
		LOG_ERR("Overwriting Header: " << field);
		LOG_ERR("Current headers: {\n" << getHeadersAsString() << "}");
	}

	strToLower(copy);
	m_headers[copy] = value;
}

bool HTTP::hasHeader(const std::string& field) const {
	std::string copy(field);

	strToLower(copy);
	return m_headers.find(copy) != m_headers.end();
}

std::string& HTTP::getBody() {
	return m_body;
}

const std::string& HTTP::getBody() const {
	return m_body;
}

std::string HTTP::getHeaderValue(const std::string& field) const {
	std::string copy(field);

	strToLower(copy);
	return m_headers.find(copy)->second;
}

std::string HTTP::getHeadersAsString() const {
	std::string headers;

	for (MapIter it = m_headers.begin(); it != m_headers.end(); it++)
		headers += capitalizeFieldPretty(it->first) + ": " + it->second + CRLF;
	return headers;
}
