#include "AutoIndex.hpp"
#include "Response.hpp"
#include "buffer.hpp"  // buf, BUFFER_SIZE
#include "defines.hpp" // CRLF
#include "logger.hpp"
#include "utils.hpp" // toHex

#include <string>

void Response::trimChunk(ssize_t bytes_sent) {
	m_chunk.erase(0, bytes_sent);
}

const std::string &Response::getNextChunk() {
	if (!m_doneReading && m_chunk.size() < BUFFER_SIZE) {
		if (m_isCGI) {
			if (!m_headersDone)
				getCGIHeaderChunk();
			else {
				if (m_isChunked)
					encodeChunked(m_saved);
				m_chunk += m_saved;
				m_saved.clear();
			}
		} else {
			readFromFile();
		}
	}
	return m_chunk;
}

void Response::encodeChunked(std::string &str) {
	str.insert(0, toHex(str.length()) + CRLF);
	str += CRLF;
}

void Response::createIndex(const std::string &path_to_index) {
	Entry		root  = { m_request.getLocation(), recursivePathCount(path_to_index) };
	std::string title = "Index of directory: " + root.name;

	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html>\n");
	addToBody("<head><meta charset='utf-8'><title>" + title + "</title></head>\n");
	addToBody("<h1>" + title + "</h1>\n<ul>");

	for (auto &entry : root.subdir)
		addToBody(entry.toString());

	addToBody("</ul></body></html>");

	m_doneReading = true;
	m_status	  = 200;

	m_chunk = getResponseAsString();
}

short Response::readFromFile() {
	ssize_t bytes_read = ::read(m_source_fd, buf, BUFFER_SIZE);

	LOG(CYAN "Read: " DEFAULT << bytes_read);
	switch (bytes_read) {
		case -1:
			perror("read");
		case 0:
			m_doneReading = true;
			break;
		default:
			// LOG(YELLOW << std::string(winSize(), '-'));
			// LOG(std::string(buf, bytes_read));
			// LOG(std::string(winSize(), '-') << DEFAULT);

			addToChunk(bytes_read);

			if (bytes_read < BUFFER_SIZE) {
				addToChunk(0);
				m_doneReading = true;
			}
	}
	return 0;
}

void Response::addToChunk(ssize_t size) {
	m_chunk += toHex(size) + CRLF;
	m_chunk.append(buf, size);
	m_chunk += CRLF;
}
