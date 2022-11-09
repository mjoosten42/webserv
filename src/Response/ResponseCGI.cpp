#include "Response.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include "utils.hpp" // fatal_perror

#include <string>
#include <unistd.h> // write

void Response::handleCGI() {
	LOG(RED "Handle CGI: " DEFAULT + m_filename);

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
	ssize_t		 bytes_written = write(m_cgi.popen.writefd, body.data(), body.length());

	LOG(RED "Write: " DEFAULT << bytes_written);

	if (bytes_written == -1) // TODO
		LOG_ERR("write: " << strerror(errno) << ": " << m_cgi.popen.writefd);
	else
		body.erase(0, bytes_written);

	if (m_request.getState() == DONE && body.empty())
		close(m_cgi.popen.writefd);
}
