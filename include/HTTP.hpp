#pragma once

#include <map>
#include <string>

class HTTP {
	public:
		HTTP();

		const std::string						& getBody() const;
		const std::map<std::string, std::string>& getHeaders() const;

		void reset();

	protected:
		std::string						   m_total;
		std::size_t						   m_pos;
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
};
