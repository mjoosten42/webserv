#pragma once

#include <map>
#include <string>

class HTTP {
	public:
		HTTP();

		void stringToData();

		void reset();

		std::string& getBody();

	protected:
		std::string	 getNextLine();
		std::size_t	 newLineLength(std::size_t pos);
		virtual void parseStartLine() = 0;
		void		 parseHeaders();

	protected:
		std::string						   m_total;
		std::size_t						   m_pos;
		std::map<std::string, std::string> m_headers;
		std::string						   m_body;
};
