#include "MIME.hpp" // fromFileName
#include "Response.hpp"
#include "buffer.hpp" // BUFFER_SIZE
#include "file.hpp"	  // isDir
#include "logger.hpp"
#include "stringutils.hpp" // toString<size_t>
#include "syscalls.hpp"	   // WS::realpath

#include <fcntl.h> // O_RDONLY
#include <string>

void Response::processRequest() {
	m_processedRequest = true;

	initialize();

	if (!m_request.isGood())
		return sendFail(m_request.getStatus(), m_request.getErrorMsg());

	if (!m_server->allowsMethod(m_locationIndex, m_request.getMethod()))
		return sendFail(405, "Allowed methods: " + m_server->getAllowedMethodsAsString(m_locationIndex));

	size_t cmb = m_server->getCMB(m_locationIndex);
	if (cmb && m_request.getContentLength() > cmb)
		return sendFail(413, "Max body size is " + toString(cmb));

	if (m_server->isRedirect(m_locationIndex))
		return sendMoved(m_server->getRedirect(m_locationIndex));

	if (isDir(m_filename))
		return sendMoved(m_request.getLocation() + '/');

	try {
		if (m_isCGI)
			handleCGI();
		else
			handleFile();
	} catch (int error) {
		m_status = error;
		sendFail(error, getStatusMessage());
	}
}

void Response::handleFile() {
	if (m_request.getMethod() != GET)
		return sendFail(405, toString(m_request.getMethod()) + " not allowed on static file");

	int fd = open(m_filename.c_str(), O_RDONLY);

	if (fd == -1)
		return openError();

	m_source_fd = fd;
	addFileHeaders();
	m_status = 200;
	m_chunk	 = getResponseAsString();
}

void Response::openError() {
	std::string dir		  = m_server->getRoot(m_locationIndex) + m_request.getLocation();
	bool		autoIndex = m_server->isAutoIndex(m_locationIndex);

	switch (errno) {
		case EACCES:
			throw 403;
		case ENOENT:
		case ENOTDIR:
			if (isDir(dir) && autoIndex)
				return createIndex(dir);
			throw 404;
		case EMFILE:
			throw 503;
		default:
			throw 500;
	}
}

void Response::addFileHeaders() {
	off_t size = fileSize(m_source_fd);

	addHeader("Content-Type", MIME::fromFileName(m_filename));

	m_isChunked = (size > BUFFER_SIZE);

	if (m_isChunked)
		addHeader("Transfer-Encoding", "Chunked");
	else
		addHeader("Content-Length", toString(size));
}
