#include "HTTP.hpp"

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
