#include "Response.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "utils.hpp"

void Response::serveError(const std::string& str) {
	m_isCGI = false; // when we have an error, the CGI is no longer active.
	sendFail(m_statusCode, str);
}

void Response::sendFail(int code, const std::string& msg) {
	auto errorPages = m_server->getErrorPages();

	if (errorPages.find(code) != errorPages.end()) {
		m_filename	  = errorPages[code];
		m_doneReading = false;
		handleFile();
		m_statusCode = 200;
		return;
	}

	m_doneReading = true;

	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(code) + " " + getStatusMessage() + "</title></head>");
	addToBody("<body><h1>" + toString(code) + " " + getStatusMessage() + "</h1>" CRLF);
	addToBody("<p>oops something went wrong: <b>" + msg + "</b></p></body></html>" CRLF);

	m_chunk = getResponseAsString();
}

void Response::sendMoved(const std::string& location) {
	m_statusCode  = 301;
	m_doneReading = true;

	addHeader("Location", location);
	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(m_statusCode) + " " + getStatusMessage() + "</title></head>");
	addToBody("<h1>" + toString(m_statusCode) + " " + getStatusMessage() + "</h1>" CRLF);
	addToBody("<p>The resource has been moved to <a href=\"" + location + "\">" + location + "</a>.</p>" CRLF);

	m_chunk = getResponseAsString();
}
