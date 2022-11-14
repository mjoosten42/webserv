#include "Response.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "utils.hpp"

// By the time sendFail or sendMoved is called, the error should be known and m_statusCode should be set.

bool Response::sendCustomErrorPage() {
	if (m_server->getErrorPages().find(m_statusCode) != m_server->getErrorPages().end()) {
		m_filename	  = m_server->getErrorPages().at(m_statusCode);
		m_doneReading = false;
		handleFile();
		return true;
	}
	return false;
}

void Response::sendFail(const std::string& msg) {
	m_isCGI = false; // when we have an error, the CGI is no longer active.
	if (sendCustomErrorPage())
		return;
	m_doneReading = true;

	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(m_statusCode) + " " + getStatusMessage() + "</title></head>");
	addToBody("<body><h1>" + toString(m_statusCode) + " " + getStatusMessage() + "</h1>" CRLF);
	addToBody("<p>oops something went wrong: <b>" + msg + "</b></p></body></html>" CRLF);

	m_chunk = getResponseAsString();
}

void Response::sendMoved(const std::string& address) {
	LOG("SEND MOVED");
	LOG("Address: " + address);
	m_statusCode = 301; // TODO: should be superfluous
	if (sendCustomErrorPage())
		return;
	m_doneReading = true;
	m_request.setLocation(m_server->getRedirect(address));
	std::string redirect = m_request.getLocation();

	addHeader("Location", redirect);
	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(m_statusCode) + " " + getStatusMessage() + "</title></head>");
	addToBody("<h1>" + toString(m_statusCode) + " " + getStatusMessage() + "</h1>" CRLF);
	addToBody("<p>The resource has been moved to <a href=\"" + redirect + "\">" + redirect + "</a>.</p>" CRLF);

	m_chunk = getResponseAsString();
}
