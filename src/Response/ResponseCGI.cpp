#include "Response.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include "syscalls.hpp"

#include <string>

void Response::handleCGI() {
	m_cgi.start(m_request, m_server, m_filename, m_peer);

	m_source_fd = m_cgi.popen.readfd;

	if (m_request.getMethod() == GET)
		m_statusCode = 200;
	else
		m_statusCode = 201;
}

// TODO: CGI Status header
void Response::getCGIHeaderChunk() {
	std::string line;

	m_saved += readBlockFromFile();

	// Parse headers line by line
	while (containsNewline(m_saved)) {
		line = getNextLine();
		if (line.empty()) {
			m_CGI_DoneProcessingHeaders = true;
			break;
		}
		try {
			parseHeader(line);
		} catch (const ServerException& e) {
			m_statusCode = e.code;
			serveError(e.what());
			break;
		}
	}

	if (m_doneReading && !m_CGI_DoneProcessingHeaders) { // CGI exited before completing respones
		m_saved.clear();
		m_statusCode = 502;
		serveError("CGI exited before completing headers");
	}

	if (m_CGI_DoneProcessingHeaders) {
		addHeader("Transfer-Encoding", "Chunked");
		m_chunk = getResponseAsString();
		encodeChunked(m_saved);
		m_chunk += m_saved;
		m_saved.clear();
	}
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
		default:
			body.erase(0, bytes_written);
	}
}
