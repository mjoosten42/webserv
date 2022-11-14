#include "Response.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "utils.hpp"

// By the time serveError is called, the error should be known and m_statusCode should be set. 
void Response::serveError(const std::string& str) {
	m_isCGI = false; // when we have an error, the CGI is no longer active.
	sendFail(str);
}

bool Response::sendCustomErrorPage(){
	if (m_server->getErrorPages().find(m_statusCode) != m_server->getErrorPages().end()) {
		m_filename	  = m_server->getErrorPages().at(m_statusCode);
		m_doneReading = false;
		handleFile();
		return true;
	}
	return false;
}

void Response::sendFail(const std::string& msg) {
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

void Response::sendMoved(const std::string& location) {
	m_statusCode  = 301; // should be superfluous
	if (sendCustomErrorPage())
		return;
	m_doneReading = true;

	addHeader("Location", location);
	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(m_statusCode) + " " + getStatusMessage() + "</title></head>");
	addToBody("<h1>" + toString(m_statusCode) + " " + getStatusMessage() + "</h1>" CRLF);
	addToBody("<p>The resource has been moved to <a href=\"" + location + "\">" + location + "</a>.</p>" CRLF);

	m_chunk = getResponseAsString();
}
