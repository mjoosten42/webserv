#include "Response.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include "syscalls.hpp"

#include <string>
#include <unistd.h> // access

void Response::handleCGI() {
	if (access(m_filename.c_str(), F_OK) == -1)
		throw 404;
	if (access(m_filename.c_str(), X_OK) == -1)
		throw 403;

	m_cgi.start(*this);

	m_source_fd = m_cgi.popen.readfd;
	m_hadFD		= true;

	if (m_request.getMethod() == GET)
		m_status = 200;
	else
		m_status = 201;
}

void Response::getCGIHeaderChunk() {
	parseCGIHeaders();

	if (m_doneReading && !m_headersDone) // CGI exited before completing respones
		return sendFail(502, "CGI exited before completing headers");

	if (m_headersDone) // CGI finished sending headers
		processCGIHeaders();
}

// Parse headers line by line
void Response::parseCGIHeaders() {
	std::string line;

	while (containsNewline(m_saved)) {
		line = getNextLine();
		if (line.empty()) {
			m_headersDone = true;
			return;
		}
		try {
			parseHeader(line);
		} catch (const ServerException &e) {
			LOG("CGI header exception: " << line);
			return sendFail(e.code, e.what());
		}
	}
}

void Response::processCGIHeaders() {
	if (!hasHeader("Content-Length") && !hasHeader("Transfer-Encoding")) {
		addHeader("Transfer-Encoding", "Chunked");
		m_isChunked = true;
	}

	if (!m_saved.empty()) {
		if (m_isChunked)
			encodeChunked(m_saved);
		addToBody(m_saved);
		m_saved.clear();
	}

	m_chunk = getResponseAsString();
}
