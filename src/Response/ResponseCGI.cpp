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

	m_cgi.start(m_request, m_server, m_filename, m_peer);

	m_source_fd = m_cgi.popen.readfd;

	if (m_request.getMethod() == GET)
		m_status = 200;
	else
		m_status = 201;
}

// TODO: CGI Status header
void Response::getCGIHeaderChunk() {
	std::string line;

	m_saved += readBlock();

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
			LOG("CGI header exception: " << line);
			sendFail(e.code, e.what());
			break;
		}
	}

	if (m_doneReading && !m_CGI_DoneProcessingHeaders) // CGI exited before completing respones
		return sendFail(502, "CGI exited before completing headers");

	if (m_CGI_DoneProcessingHeaders) { // CGI finished sending headers
		if (!hasHeader("Content-Length") && !hasHeader("Transfer-Encoding")) {
			addHeader("Transfer-Encoding", "Chunked");
			m_isChunked = true;
		}
		if (!m_saved.empty()) {
			std::string block = m_saved;
			if (m_isChunked)
				encodeChunked(block);
			addToBody(block);
		}
		m_chunk = getResponseAsString();
		m_saved.clear();
	}
}
