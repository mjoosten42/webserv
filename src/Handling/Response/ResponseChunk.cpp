#include "AutoIndex.hpp"
#include "Response.hpp"
#include "buffer.hpp"  // BUFFER_SIZE
#include "defines.hpp" // CRLF
#include "utils.hpp"   // toHex

#include <string>

void Response::trimChunk(ssize_t bytes_sent) {
	m_chunk.erase(0, bytes_sent);
}

bool Response::isDone() const {
	return m_doneReading && m_chunk.empty();
}

std::string &Response::getNextChunk() {
	if (!m_doneReading && m_chunk.size() < BUFFER_SIZE) {
		m_saved += readBlock();
		if (m_isCGI && !m_headersDone)
			getCGIHeaderChunk();
		else {
			if (m_isChunked)
				encodeChunked(m_saved);
			m_chunk += m_saved;
			m_saved.clear();
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
