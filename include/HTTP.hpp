#pragma once

#include <map>
#include <string>

class HTTP {
	public:
		HTTP();

		const std::string						& getBody() const;
		const std::map<std::string, std::string>& getHeaders() const;

		void addToBody(const std::string& str);
		void addHeader(const std::string& field, const std::string& value);

		void reset();

	protected:
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
};
