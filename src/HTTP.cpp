#include "HTTP.hpp"

#include "utils.hpp"

#include <sstream>

//  Note: according to RFC, \r\n is the correct newline
//  However, it recommends parsers to also consider just \n sufficient
//  HTTP 1.1 mandatory [port is optional]
//  	GET / HTTP/1.1
//  		Host: localhost:8080

HTTP::HTTP(): m_pos(0) {}

void HTTP::stringToData() {
	parseStartLine();
	parseHeaders();

	m_total.erase(0, m_pos); //  Cut of start-line and headers, leaving only the body
	m_total.swap(m_body);	 //  No copying needed

	printStringMap(m_headers);
	std::cout << "Body: {\n" << m_body << "}\n";
}

std::string HTTP::getNextLine() {
	std::size_t pos = m_total.find_first_of(CRLF, m_pos); //  find index of next newline
	if (pos == std::string::npos)						  //  EOF
		return "";
	std::string line = m_total.substr(m_pos, pos - m_pos); //  extract the substring between old newline and new newline
	m_pos			 = pos + newLineLength(pos);		   //  update old newline, skipping \n or \r\n
	return line;
}

std::size_t HTTP::newLineLength(std::size_t pos) {
	if (m_total[pos] == '\n')
		return 1;
	if (m_total[pos + 1] == '\n') //  m_total[pos] must be \r
		return 2;
	return -1;
}

std::string HTTP::testMethod(const std::string& str) const {
	const static char *table[] = { "GET", "POST", "DELETE" };
	const static int   size	   = sizeof(table) / sizeof(*table);

	for (int i = 0; i < size; i++)
		if (str == table[i])
			return str;
	return "";
}

void HTTP::parseHeaders() {
	std::pair<std::map<std::string, std::string>::iterator, bool> insert;
	std::pair<std::string, std::string>							  header;
	std::istringstream											  line(getNextLine());

	line >> header.first;
	line >> header.second;
	while ((header.first != CRLF || header.first != "\n") && !header.first.empty()) {
		strToUpper(header.first);
		if (header.first.back() != ':')
			std::cerr << "Header field must end in ':' : " << header.first << std::endl;
		insert = m_headers.insert(header);
		if (!insert.second)
			std::cerr << "Duplicate headers: " << header.first << std::endl;
		line.clear();
		line.str(getNextLine());
		header.first.clear();
		line >> header.first;
		line >> header.second;
		if (header.first.empty())
			break;
	}
}
