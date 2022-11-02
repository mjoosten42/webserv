#include "MIME.hpp" // TODO: remove
#include "Response.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <string>

void Response::getCGIHeaderChunk() {

	std::string block = readBlockFromFile();

	size_t pos = findHeaderEnd(block);
	if (pos == std::string::npos) {
		m_chunk += block;
	} else {
		m_CGI_DoneProcessingHeaders = true;
		std::string headers			= block.substr(0, pos);
		block						= block.substr(pos);

		if (block.empty()) // Only send one trailing chunk
			m_doneReading = true;

		encodeChunked(block);
		m_chunk += headers + block;
	}
}

void Response::handleCGI() {
	LOG(RED "Handle CGI: " DEFAULT + m_filename);

	// TODO: parse from config
	std::string bin = "/usr/bin/php";
	if (getExtension(m_request.getLocation()) == "pl")
		bin = "/usr/bin/pl";

	m_statusCode = m_cgi.start(m_request, m_server, bin, m_filename);

	if (m_statusCode == 200) {
		addHeader("Transfer-Encoding", "Chunked");
		m_source_fd					= m_cgi.popen.readfd;
		m_CGI_DoneProcessingHeaders = false;

		m_chunk = getStatusLine() + getHeadersAsString();
	} else {
	} // TODO
}

void Response::writeToCGI() {
	std::string& body		   = m_request.getBody();
	ssize_t		 bytes_written = write(m_cgi.popen.writefd, body.data(), body.length());

	LOG(RED "Write: " DEFAULT << bytes_written);

	if (bytes_written == -1)
		fatal_perror("write"); // TODO
	body.erase(0, bytes_written);

	if (m_request.getState() == DONE && body.empty())
		close(m_cgi.popen.writefd);
}
