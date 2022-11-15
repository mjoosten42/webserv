#include "Response.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "utils.hpp"

// SendFail and SendMoved set m_status themselves, to save lines

void Response::sendCustomErrorPage() {
	int tmp = m_status;

	m_filename	  = m_server->getErrorPage(m_status);
	m_doneReading = false;
	handleFile();
	m_status = tmp;
	m_chunk	 = getResponseAsString(); // Overwriting old data
}

void Response::sendFail(int code, const std::string& msg) {
	m_status = code;
	m_isCGI	 = false; // when we have an error, the CGI is no longer active.
	if (m_server->hasErrorPage(m_status))
		return sendCustomErrorPage();
	m_doneReading = true;

	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(m_status) + " " + getStatusMessage() + "</title></head>");
	addToBody("<body><h1>" + toString(m_status) + " " + getStatusMessage() + "</h1>" CRLF);
	addToBody("<p>oops something went wrong: <b>" + msg + "</b></p></body></html>" CRLF);

	m_chunk = getResponseAsString();
}

void Response::sendMoved(const std::string& address) {
	LOG("SEND MOVED");
	LOG("Address: " + address);
	m_status = 301; // TODO: should be superfluous
	if (m_server->hasErrorPage(m_status))
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
