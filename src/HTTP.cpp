#include "HTTP.hpp"

#include <string>

HTTP::HTTP(): m_pos(0) {}

void HTTP::reset() {
	m_headers.clear();
	m_total.clear();
	m_body.clear();
	m_pos = 0;
}

const std::string& HTTP::getBody() const {
	return m_body;
}

const std::map<std::string, std::string>& HTTP::getHeaders() const {
	return m_headers;
}
