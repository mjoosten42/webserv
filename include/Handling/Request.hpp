#pragma once

#include "HTTP.hpp"
#include "methods.hpp"

#include <string>

enum state { STARTLINE, HEADERS, BODY, DONE };

class Request: public HTTP {
	public:
		Request();

		void append(const char *buf, ssize_t size);

		state	getState() const;
		methods getMethod() const;

		const std::string &getHost() const;
		const std::string &getLocation() const;
		const std::string &getQueryString() const;
		const std::string &getPathInfo() const;
		const std::string &getErrorMsg() const;

		size_t getContentLength() const;
		size_t getBodyTotal() const;

		std::string getStateAsString() const;

	private:
		void parse();
		void checkSpecialHeaders();

		void parseStartLine(const std::string &line);
		void parseMethod(const std::string &str);
		void parseURI(const std::string &str);
		void parseHTTPVersion(const std::string &str);

	private:
		state		m_state;
		methods		m_method;	   // GET, POST, DELETE
		std::string m_location;	   // ex. /foo/bar.html
		std::string m_queryString; // ex. amongus=sus&greeting=Good%20morning
		std::string m_pathInfo;	   // /info.php/another/path

		std::string m_host;
		size_t		m_contentLength;
		size_t		m_bodyTotal;

		std::string m_errorMsg;

		// for the unit tester
		friend class TesterHelper;
};
