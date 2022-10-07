#pragma once

#include "Server.hpp"

#include <map>
#include <string>

class Request {
	public:
		Request(int fd, const Server *server);

		void add(const std::string& str);
		void parse();

	private:
		std::string getNextLine();
		std::size_t newLineLength(std::size_t pos);
		std::string testMethod(const std::string& str);
		void		parseStartLine();
		void		parseHeaders();

	private:
		int			  m_fd;
		const Server *m_server;

		std::string m_total;
		std::size_t m_pos;

		std::string						   m_location;
		std::string						   m_method;
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
};
