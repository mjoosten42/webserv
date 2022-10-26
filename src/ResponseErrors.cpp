#include "Response.hpp"
#include "Server.hpp"
#include "utils.hpp"

int Response::serveError(int code) {
	sendFail(code, "This is a placeholder message.\n");
	return (code);
}

void Response::sendFail(int code, const std::string& msg) {
	m_statusCode = code;
	if (m_server->getErrorPages().find(code) != m_server->getErrorPages().end()) {
		std::string file = m_server->getErrorPages().at(code);
		handleGetWithStaticFile(file);
		return;
	}
	addDefaultHeaders();
	addHeader("Content-Type", "text/html");

	addToBody("<h1>" + toString(code) + " " + getStatusMessage() + "</h1>\r\n");
	addToBody("<p>something went wrong somewhere: <b>" + msg + "</b></p>\r\n");

	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}

void Response::sendMoved(const std::string& location) {
	clear(); //  <!-- TODO, also add default server
	addDefaultHeaders();
	m_statusCode			  = 301;
	m_headers["Location"]	  = location;
	m_headers["Connection"]	  = "Close";
	m_headers["Content-Type"] = "text/html";

	addToBody("<h1>" + toString(m_statusCode) + " " + getStatusMessage() + "</h1>\r\n");
	addToBody("<p>The resource has been moved to <a href=\"" + location + "\">" + location + "</a>.</p>");

	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}
