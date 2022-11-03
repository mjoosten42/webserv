#include "Response.hpp"
#include "Server.hpp"
#include "utils.hpp"

void Response::serveError(const std::string& str) {
	m_isCGI = false; // when we have an error, the CGI is no longer active.
	sendFail(m_statusCode, str);
}

void Response::sendFail(int code, const std::string& msg) {

	if (m_server->getErrorPages().find(code) != m_server->getErrorPages().end()) {
		std::string file = m_server->getErrorPages().at(code);
		m_filename		 = file;
		m_doneReading	 = false;
		handleFile();
		return;
	}

	m_doneReading = true;

	addHeader("Content-Type", "text/html");

	addToBody("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>");
	addToBody("<title>" + toString(code) + " " + getStatusMessage() + "</title></head>");
	addToBody("<body><h1>" + toString(code) + " " + getStatusMessage() + "</h1>\r\n");
	addToBody("<p>oops something went wrong: <b>" + msg + "</b></p></body></html>\r\n");
}

void Response::sendMoved(const std::string& location) {
	m_statusCode  = 301;
	m_doneReading = true;

	// clear(); // <!-- TODO, also add default server
	addHeader("Location", location);
	addHeader("Content-Type", "text/html");

	addToBody("<h1>" + toString(m_statusCode) + " " + getStatusMessage() + "</h1>\r\n");
	addToBody("<p>The resource has been moved to <a href=\"" + location + "\">" + location + "</a>.</p>");
}
