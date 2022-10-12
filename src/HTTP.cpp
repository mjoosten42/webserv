#include "HTTP.hpp"

#include "defines.hpp"

#include <string>

HTTP::HTTP() {}

void HTTP::reset() {
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

void HTTP::addHeader(const std::string& field, const std::string& value) {
	m_headers[field] = value;
}
