#include "Response.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "utils.hpp"

// SendFail and SendMoved set m_status themselves, to save lines

void Response::sendCustomErrorPage() {
	int tmp = m_status;

	m_filename	  = m_server->getRoot(m_locationIndex) + m_server->getErrorPage(m_locationIndex, m_status);
	m_doneReading = false;
	handleFile(); // Will set status to 200 if successful
	m_status = tmp;
	m_chunk	 = getResponseAsString(); // Overwriting old data
}

void Response::sendFail(int code, const std::string &msg) {
	m_status = code;
	m_isCGI	 = false; // when we have an error, the CGI is no longer active.

	if (m_server->hasErrorPage(m_locationIndex, m_status)) {
		try {
			return sendCustomErrorPage();
		} catch (int error) {
			m_status = error;
		}
	}

	m_doneReading = true;

	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(m_status) + " " + getStatusMessage() + "</title></head>");
	addToBody("<body><h1>" + toString(m_status) + " " + getStatusMessage() + "</h1>" CRLF);
	addToBody("<p>Oops something went wrong: <b>" + msg + "</b></p></body></html>" CRLF);

	m_chunk = getResponseAsString();
}

void Response::sendMoved(const std::string &address) {
	m_status = 301;
	if (m_server->hasErrorPage(m_locationIndex, m_status))
		return sendCustomErrorPage();
	m_doneReading = true;

	addHeader("Location", address);
	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(m_status) + " " + getStatusMessage() + "</title></head>");
	addToBody("<h1>" + toString(m_status) + " " + getStatusMessage() + "</h1>" CRLF);
	addToBody("<p>The resource has been moved to <a href=\"" + address + "\">" + address + "</a>.</p>" CRLF);

	m_chunk = getResponseAsString();
}
