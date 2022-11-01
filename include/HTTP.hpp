#pragma once

#include <map>
#include <string>

class HTTP {
	public:
		HTTP();

		static std::string capitalizeFieldPretty(std::string field);

		std::string								& getBody();
		const std::string						& getBody() const;
		const std::map<std::string, std::string>& getHeaders() const;

		void addToBody(const char *buf, ssize_t size);
		void addToBody(const std::string& str);
		void addHeader(const std::string field, const std::string& value);
		bool hasHeader(const std::string& field) const;

		std::string getHeaderValue(const std::string& field) const;
		std::string getHeadersAsString() const;

		void clear();

	protected:
		std::map<std::string, std::string> m_headers; // the key value pair. Note: keys should always be lowercase!
		std::string						   m_body;
};
