#pragma once

#include <map>
#include <string>

class HTTP {
	public:
		HTTP();

	protected:
		std::string						   m_total;
		std::size_t						   m_pos;
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
};
