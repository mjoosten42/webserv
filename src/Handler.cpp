#include "Handler.hpp"

#include "GetStaticFileTransfer.hpp" //TODO: remove
#include "Server.hpp"
#include "shared_fd.hpp"
#include "utils.hpp"

#include <fstream>
#include <iostream> // TODO: remove
#include <string>
#include <sys/socket.h> // send

Handler::Handler(): m_fd(make_shared(-1)), m_server(NULL) {}

Handler::Handler(int fd, const Server *server): m_fd(make_shared(fd)), m_server(server) {
	(void)m_server;
}

void Handler::reset() {
	m_request.reset();
	m_response.reset();
}

void Handler::handle() {
	switch (m_request.getMethod()) {
		case GET:
			handleGet();
			break;
		case POST:
			break;
		case DELETE:
			break;
		default:
			std::cerr << "Error: method is NONE\n";
	}
}

void Handler::handleGet() {
	m_response.setStatusCode(handleGetWithStaticFile(m_fd, m_request.getLocation()));

	switch (m_response.getStatusCode()) {
		case 404:
			sendFail(404, "Opening file failed whoops");
			break;
		case 200:

		default:
			break;
	}
}

#include "MIME.hpp"

int Handler::handleGetWithStaticFile(const int socket_fd, const std::string& filename) {
	std::ifstream infile("." + filename, std::ios::in | std::ios::binary); //  TODO: remove dot

	if (!infile.is_open())
		return 404;

	std::string headers = std::string("HTTP/1.1 200 OK\r\n"
									  "Connection: Keep-Alive\r\n"
									  "Content-Type: ");
	headers += MIME::fromFileName(filename) + "\r\n";

	transferFile(socket_fd, infile, headers);
	return 200;
}

//  very much temporary
//  for the future: server also needs to be passed, as that may have custom 404 pages etc.
void Handler::sendFail(int code, const std::string& msg) {
	m_response.addToBody("<h1>" + std::to_string(code) + " " + m_response.getStatusMessage() + "</h1>\r\n");
	m_response.addToBody("<p>something went wrong somewhere: <b>" + msg + "</b></p>\r\n");

	m_response.addHeader("Content-Type", "text/html");
	m_response.addHeader("Content-Length", std::to_string(m_response.getBody().length()));

	std::string response = m_response.getResponseAsCPPString();
	if (send(m_fd, response.c_str(), response.length(), 0) == -1)
		fatal_perror("send");
}
