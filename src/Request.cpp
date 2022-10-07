#include "Request.hpp"

#include "utils.hpp"

#include <fstream>
#include <iostream> // TODO:remove
#include <sstream>
#include <string>
#include <sys/socket.h> // send()

#define CRLF "\r\n"

//  Note: according to RFC, \r\n is the correct newline
//  However, it recommends parsers to also condsider just \n sufficient
//  http1.1 mandatory [port is optional]
//  	GET / HTTP/1.1
//  		Host: localhost:8080

Request::Request(int fd, const Server *server): m_fd(fd), m_server(server), m_pos(0) {
	(void)m_server;
	(void)m_fd;
}

void Request::add(const std::string& str) {
	m_total += str;
}

void Request::parse() {
	parseStartLine();
	parseHeaders();

	m_total.erase(0, m_pos); //  Cut of start-line and headers, leaving only the body
	m_total.swap(m_body);	 //  No copying needed

	//  printMethod(m_method);
	//  std::cout << "Location: " << m_location << std::endl;
	//  printStringMap(m_headers);
	//  std::cout << "Body: {\n" << m_body << "}\n";
}

std::string Request::getNextLine() {
	std::size_t pos = m_total.find_first_of(CRLF, m_pos); //  find index of next newline
	if (pos == std::string::npos)						  //  EOF
		return std::string();
	std::string line = m_total.substr(m_pos, pos - m_pos); //  extract the substring between old newline and new newline
	m_pos			 = pos + newLineLength(pos);		   //  update old newline, skipping \n or \r\n
	return line;
}

std::size_t Request::newLineLength(std::size_t pos) {
	if (m_total[pos] == '\n')
		return 1;
	if (m_total[pos + 1] == '\n') //  m_total[pos] must be \r
		return 2;
	return -1;
}

methods Request::testMethod(const std::string& str) {
	const static char *table[] = { "GET", "POST", "DELETE" };
	const static int   size	   = sizeof(table) / sizeof(*table);

	for (int i = 0; i < size; i++)
		if (str == table[i])
			return static_cast<methods>(i);
	return NONE;
}

void Request::parseStartLine() {
	std::istringstream line(getNextLine());
	std::string		   word;

	line >> word;
	if ((m_method = testMethod(word)) == NONE)
		std::cerr << "Incorrect method: " << word << std::endl;

	line >> m_location;
	if (m_location.empty())
		std::cerr << "Empty location: " << m_location << std::endl;

	word.clear();
	line >> word;
	if (word != "HTTP/1.1")
		std::cerr << "HTTP 1.1 only: " << word << std::endl;
}

void Request::parseHeaders() {
	std::pair<std::map<std::string, std::string>::iterator, bool> insert;
	std::pair<std::string, std::string>							  header;
	std::istringstream											  line(getNextLine());

	line >> header.first;
	line >> header.second;
	while ((header.first != CRLF || header.first != "\n") && !header.first.empty()) {
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

	/* https://www.rfc-editor.org/rfc/rfc7230#section-3.1
	**	"Each header field consists of a case-insensitive field name followed
	**	by a colon (":"), optional leading whitespace, the field value, and
	**	optional trailing whitespace."
	**
	**	Thus, no whitespace between field name and colon
	*/
}
