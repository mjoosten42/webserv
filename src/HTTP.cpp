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

const std::map<std::string, std::string>& HTTP::getHeaders() const {
	return m_headers;
}

void HTTP::addToBody(const std::string& str) {
	m_body += str + CRLF;
}

void HTTP::addToBody(const char *buf, std::size_t size) {
	m_body.append(buf, size);
}

void HTTP::addHeader(const std::string& field, const std::string& value) {
	m_headers[field] = value;
}

bool HTTP::hasHeader(const std::string& field) const {
	return m_headers.find(field) != m_headers.end();
}

std::string HTTP::getHeaderValue(const std::string& field) {
	return m_headers.find(field)->second;
}
