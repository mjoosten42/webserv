#include "Handler.hpp"

#include "MIME.hpp"
#include "Server.hpp"
#include "shared_fd.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <fstream>
#include <iostream> // TODO: remove
#include <sstream>
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
	m_response.setStatusCode(200);
	handleGetWithStaticFile(m_request.getLocation());

	if (m_response.getStatusCode() != 200)
		sendFail(m_response.getStatusCode(), "Page is venting");
}

int Handler::handleGetWithStaticFile(const std::string& filename) {
	std::ifstream infile("." + filename, std::ios::in | std::ios::binary); //  TODO: remove dot

	if (!infile.is_open())
		return 404;

	m_response.addHeader("Connection", "Keep-Alive");
	m_response.addHeader("Content-Type", MIME::fromFileName(filename));

	return transferFile(infile);
}

void Handler::sendFail(int code, const std::string& msg) {
	m_response.setStatusCode(code);

	m_response.addHeader("Content-Type", "text/html");

	m_response.addToBody("<h1>" + toString(code) + " " + m_response.getStatusMessage() + "</h1>\r\n");
	m_response.addToBody("<p>something went wrong somewhere: <b>" + msg + "</b></p>\r\n");

	sendResponse();
}

void Handler::sendMoved(const std::string& location) {
	m_response.reset(); //  <!-- TODO, also add default server
	m_response.setStatusCode(301);
	m_response.addHeader("Location", location);
	m_response.addHeader("Connection", "Close");

	m_response.addHeader("Content-Type", "text/html");

	m_response.addToBody("<h1>" + toString(m_response.getStatusCode()) + " " + m_response.getStatusMessage() +
						 "</h1>\r\n");
	m_response.addToBody("<p>The resource has been moved to <a href=\"" + location + "\">" + location + "</a>.</p>");

	sendResponse();
}

void Handler::sendResponse() {
	std::string response = m_response.getResponseAsString();
	if (send(m_fd, response.c_str(), response.length(), 0) == -1)
		fatal_perror("send");
	//  TODO: if send fails just remove the socket or something instead of fatal perror
}

#define FILE_BUF_SIZE (4096 - 1024)

//  WARNING: CHUNK_MAX_LENGTH CANNOT EXCEED 0xFFF as the length limit is hard coded.
//  However, there is no reason for such a high limit anyways, since browsers do not always support this.
#define CHUNK_MAX_LENGTH 1024
#define CHUNK_SENDING_SIZE (CHUNK_MAX_LENGTH + 3 + 2 * 2)

int Handler::sendChunked(std::ifstream& infile) {
	m_response.addHeader("Transfer-Encoding", "chunked");

	std::string response = m_response.getResponseAsString();
	if (send(m_fd, response.c_str(), response.length(), 0) == -1)
		fatal_perror("send"); //  TODO: remove those!!

	static char buf[CHUNK_SENDING_SIZE];
	size_t		size;
	size_t		buf_offset;

	while (true) {
		infile.read(buf + 5, CHUNK_MAX_LENGTH);

		if (infile.bad()) {
			std::cerr << "Reading infile failed!\n";
			return 404;
		}

		//  if we have reached EOF, then we finish the multichunked response with empty data.
		size = infile.gcount();
		if (size == 0) {
			const std::string end = "0\r\n\r\n";
			if (send(m_fd, end.c_str(), end.length(), 0) == -1)
				fatal_perror("send");
			break;
		}

		//  add the size of the chunk, and finish the buffer with CRLF
		{
			std::stringstream ss;

			ss.seekp(std::ios::beg);
			ss << std::hex << size;

			size_t size_str_len = ss.tellp();
			buf_offset			= 3 - size_str_len;

			std::memcpy(buf + buf_offset, ss.str().c_str(), size_str_len);
			std::memcpy(buf + buf_offset + size_str_len, "\r\n", 2);
			std::memcpy(buf + buf_offset + size_str_len + 2 + size, "\r\n", 2);
		}

		if (send(m_fd, buf + buf_offset, CHUNK_SENDING_SIZE - (CHUNK_MAX_LENGTH - size) - buf_offset, 0) == -1)
			fatal_perror("send"); //  TODO: EWOULDBLOCK
	}

	return 200;
}

int Handler::sendSingle(std::ifstream& infile) {
	static char buf[FILE_BUF_SIZE + 1];

	infile.read(buf, FILE_BUF_SIZE);
	if (infile.bad()) {
		std::cerr << "Reading infile failed!\n";
		if (errno == EACCES)
			return 403;
		return 404;
	}
	m_response.setStatusCode(200);
	m_response.addToBody(buf);

	sendResponse();
	return 200;
}

int Handler::transferFile(std::ifstream& infile) {
	int			   ret;
	std::streampos begin = infile.tellg();
	infile.seekg(0, std::ios::end);
	std::streampos end = infile.tellg();
	infile.seekg(0, std::ios::beg);

	if (end - begin > FILE_BUF_SIZE) {
		//  send multichunked
		ret = sendChunked(infile);
	} else {
		//  send in single buf.
		ret = sendSingle(infile);
	}
	infile.close();
	return ret;
}
