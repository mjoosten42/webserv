#include "Response.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include "syscalls.hpp"
#include "utils.hpp" // stringToIntegral

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

	m_status = 200;
}

void Response::getCGIHeaderChunk() {
	try {
		parseCGIHeaders();
	} catch (const ServerException &e) {
		return sendFail(502, e.what());
	}

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
		parseHeader(line);
	}
}

void Response::processCGIHeaders() {
	if (hasHeader("Status")) {
		std::string status = getHeader("Status");
		removeHeader("Status");
		try {
			m_status = stringToIntegral<unsigned int>(status);
		} catch (std::exception &e) {
			return sendFail(502, "Status is not a HTTP code: " + status);
		}
	}

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
