#pragma once

#include <map>
#include <string>
#include <unistd.h> //ssize_t

class HTTP {
	public:
		static std::string capitalizeFieldPretty(std::string field);

		void parseHeader(const std::string &line);

		void addToBody(const char *buf, ssize_t size);
		void addToBody(const std::string &str);
		void addHeader(const std::string &field, const std::string &value);
		bool hasHeader(const std::string &field) const;

		bool		containsNewline(const std::string &str);
		size_t		findNewline(const std::string &str, size_t begin = 0);
		std::string getNextLine();
		bool		isGood();

		int				   getStatus() const;
		std::string		  &getBody();
		const std::string &getBody() const;

		std::string getHeaderValue(const std::string &field) const;
		std::string getHeadersAsString() const;

	protected:
		std::map<std::string, std::string> m_headers; // the key value pair. Note: keys should always be lowercase!

		std::string m_body;
		std::string m_saved; // Save incomplete headers

		int m_status;

		class ServerException: public std::exception {
			public:
				ServerException(int error, const std::string &str) throw(): code(error), msg(str) {};
				~ServerException() throw() {};

				const char *what() const throw() { return msg.c_str(); }

				const int		  code;
				const std::string msg;
		};
};
