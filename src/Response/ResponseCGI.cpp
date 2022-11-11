#include "Response.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include "syscalls.hpp"
#include "utils.hpp" // fatal_perror

#include <string>

void Response::handleCGI() {
	m_cgi.start(m_request, m_server, m_filename);

	if (m_request.getMethod() == GET)
		m_statusCode = 200;
	if (m_request.getMethod() == POST)
		m_statusCode = 201;

	addHeader("Transfer-Encoding", "Chunked");

	m_source_fd = m_cgi.popen.readfd;
	m_chunk		= getStatusLine() + getHeadersAsString();
}

void Response::getCGIHeaderChunk() {
	std::string block = readBlockFromFile();

	if (m_doneReading && !m_CGI_DoneProcessingHeaders) { // CGI exited before completing respones
		m_savedLine.clear();
		m_chunk += CRLF;
		m_CGI_DoneProcessingHeaders = true;
	}

	block.insert(0, m_savedLine); // Prepend previous remaining data

	// Add lines to chunk, removing them from block as we go
	m_savedLine = getLine(block);
	while (!m_savedLine.empty()) {
		m_chunk += m_savedLine;
		if (m_savedLine == CRLF || m_savedLine == "\n") {
			m_CGI_DoneProcessingHeaders = true;
			break;
		}
		m_savedLine = getLine(block);
	}
	m_savedLine = block; // Save remaining data

	if (m_CGI_DoneProcessingHeaders) {
		encodeChunked(block);
		m_chunk += block;
	}
}

std::string Response::getLine(std::string& str) {
	size_t		pos = str.find('\n');
	std::string line;

	if (pos != std::string::npos) {
		line = str.substr(0, pos + 1);
		str.erase(0, pos + 1);
	}

	return line;
}

void Response::writeToCGI() {
	std::string& body		   = m_request.getBody();
	int			 fd			   = m_cgi.popen.writefd;
	ssize_t		 bytes_written = WS::write(fd, body);

	LOG(RED "Write: " DEFAULT << bytes_written);

	switch (bytes_written) {
		case -1:
			body.clear();
			break;
		case 0:
			if (m_request.getState() == DONE)
				WS::close(fd);
			break;
		default:
			body.erase(0, bytes_written);
	}
}
