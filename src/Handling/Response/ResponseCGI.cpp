#include "Response.hpp"
#include "Server.hpp"
#include "buffer.hpp" // buf
#include "logger.hpp"
#include "syscalls.hpp" // WS::write
#include "utils.hpp"	// stringToIntegral

#include <string>
#include <unistd.h> // access

void Response::handleCGI() {
	if (access(m_filename.c_str(), F_OK) == -1)
		throw 404;
	if (access(m_filename.c_str(), X_OK) == -1)
		throw 403;

	m_cgi.start(*this);

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
			break;
		}
		parseHeader(line);
	}
}

void Response::processCGIHeaders() {
	if (hasHeader("Status")) {
		std::string status = getHeader("Status");
		removeHeader("Status");
		if (!isStatus(status))
			return sendFail(502, "Status is not a HTTP code: " + status);
		m_status = stringToIntegral<int>(status);
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

short Response::readFromCGI() {
	int		fd		   = m_cgi.popen.readfd;
	ssize_t bytes_read = ::read(fd, buf, BUFFER_SIZE);
	short	flags	   = 0;

	LOG(CYAN "Read: " DEFAULT << bytes_read);
	switch (bytes_read) {
		case -1:
			perror("read");
		case 0:
			m_doneReading = true;
			break;
		default:

			LOG(YELLOW << std::string(winSize(), '-'));
			LOG(std::string(buf, bytes_read));
			LOG(std::string(winSize(), '-') << DEFAULT);

			m_saved.append(buf, bytes_read);

			flags |= POLLIN;
	}
	return flags;
}

short Response::writeToCGI() {
	std::string &body		   = m_request.getBody();
	int			 fd			   = m_cgi.popen.writefd;
	ssize_t		 bytes_written = ::write(fd, body.data(), body.size());
	short		 flags		   = 0;

	LOG(CYAN "Write: " DEFAULT << bytes_written);
	switch (bytes_written) {
		case -1:
			perror("write");
		case 0:
			m_cgi.popen.writefd.close();
			break;
		default:

			LOG(YELLOW << std::string(winSize(), '-'));
			LOG(body.substr(0, bytes_written));
			LOG(std::string(winSize(), '-') << DEFAULT);

			body.erase(0, bytes_written);
	}
	if (m_request.getBodyTotal() < m_request.getContentLength())
		flags |= POLLOUT;
	return flags;
}
