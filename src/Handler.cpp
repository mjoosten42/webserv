#include "Handler.hpp"

#include "MIME.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <fcntl.h> // open
#include <fstream>
#include <iostream> // TODO: remove
#include <sstream>
#include <string>
#include <sys/socket.h> // send
#include <unistd.h>		//lseek

Handler::Handler(Request& request, Response& response, const Server *server):
	m_request(request), m_response(response), m_server(server) {
	(void)m_server;
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

int handleCGI(const std::string& command, const std::string& filename);

void Handler::handleGet() {

	handleCGI("/usr/bin/perl", "printenv.pl");
	m_response.m_statusCode = 200;
	m_response.m_statusCode = handleGetWithStaticFile(m_request.getLocation());

	if (m_response.m_statusCode != 200)
		sendFail(m_response.m_statusCode, "Page is venting");
}

int Handler::handleGetWithStaticFile(const std::string& filename) {

	//  TODO: nonblock?
	//  TODO: remove dot
	int readfd = open(("." + filename).c_str(), O_RDONLY);
	if (readfd == -1) {
		//  TODO: correct error codes
		if (errno == EACCES)
			return 403;
		perror("open");
		return 404;
	}

	m_response.addHeader("Connection", "Keep-Alive");
	m_response.addHeader("Content-Type", MIME::fromFileName(filename));

	return transferFile(readfd);
}

void Handler::sendFail(int code, const std::string& msg) {
	m_response.m_statusCode = code;

	m_response.addHeader("Content-Type", "text/html");

	m_response.addToBody("<h1>" + toString(code) + " " + m_response.getStatusMessage() + "</h1>\r\n");
	m_response.addToBody("<p>something went wrong somewhere: <b>" + msg + "</b></p>\r\n");
	m_response.addHeader("Content-Length", toString(m_response.getBody().length()));
}

void Handler::sendMoved(const std::string& location) {
	m_response.clear(); //  <!-- TODO, also add default server
	m_response.m_statusCode = 301;
	m_response.addHeader("Location", location);
	m_response.addHeader("Connection", "Close");

	m_response.addHeader("Content-Type", "text/html");

	m_response.addToBody("<h1>" + toString(m_response.m_statusCode) + " " + m_response.getStatusMessage() +
						 "</h1>\r\n");
	m_response.addToBody("<p>The resource has been moved to <a href=\"" + location + "\">" + location + "</a>.</p>");
	m_response.addHeader("Content-Length", toString(m_response.getBody().length()));
}

#define FILE_BUF_SIZE (4096 - 1024)

int Handler::transferFile(int readfd) {
	int ret;

	//  get file size
	off_t size = lseek(readfd, 0, SEEK_END);
	if (size == -1)
		size = std::numeric_limits<off_t>().max();
	lseek(readfd, 0, SEEK_SET); //  set back to start

	if (size > FILE_BUF_SIZE) {
		//  send multichunked
		//  ret = sendChunked(readfd);
		ret = sendSingle(readfd);
	} else {
		//  send in single buf.
		ret = sendSingle(readfd);
	}
	close(readfd);
	return ret;
}

int Handler::sendSingle(int readfd) {
	static char buf[FILE_BUF_SIZE + 1];
	int			bytes_read = read(readfd, buf, FILE_BUF_SIZE);

	if (bytes_read == -1) {
		std::cerr << "Reading infile failed!\n";
		return 500;
	}
	buf[bytes_read]			= 0;
	m_response.m_statusCode = 200;
	m_response.addToBody(buf);
	m_response.addHeader("Content-Length", toString(m_response.getBody().length()));

	return 200;
}

//  WARNING: CHUNK_MAX_LENGTH CANNOT EXCEED 0xFFF as the length limit is hard coded.
//  However, there is no reason for such a high limit anyways, since browsers do not always support this.
#define CHUNK_MAX_LENGTH 1024
#define CHUNK_SENDING_SIZE (3 + 2 + CHUNK_MAX_LENGTH + 2)

//  int Handler::sendChunked(int readfd) {
//  	m_response.addHeader("Transfer-Encoding", "chunked");

//  	std::string response = m_response.getResponseAsString() + CRLF;
//  	if (send(m_fd, response.c_str(), response.length(), 0) == -1)
//  		fatal_perror("send"); //  TODO: remove those!!

//  	static char buf[CHUNK_SENDING_SIZE];
//  	ssize_t		size;
//  	size_t		buf_offset;

//  	while (true) {

//  		size = read(readfd, buf + 5, CHUNK_MAX_LENGTH);

//  		if (size == -1) {
//  			std::cerr << "Reading infile failed!\n";
//  			return 500;
//  		}

//  		//  if we have reached EOF, then we finish the multichunked response with empty data.
//  		if (size == 0) {
//  			const std::string end = "0\r\n\r\n";
//  			if (send(m_fd, end.c_str(), end.length(), 0) == -1)
//  				fatal_perror("send");
//  			break;
//  		}
//  	}

//  	return 200;
//  }

//  		//  add the size of the chunk, and finish the buffer with CRLF
//  		{
//  			std::stringstream ss;

//  			ss.seekp(std::ios::beg);
//  			ss << std::hex << size;

//  			size_t size_str_len = ss.tellp();
//  			buf_offset			= 3 - size_str_len;

//  			std::memcpy(buf + buf_offset, ss.str().c_str(), size_str_len);
//  			std::memcpy(buf + buf_offset + size_str_len, "\r\n", 2);
//  			std::memcpy(buf + buf_offset + size_str_len + 2 + size, "\r\n", 2);
//  		}

//  		if (send(m_fd, buf + buf_offset, CHUNK_SENDING_SIZE - (CHUNK_MAX_LENGTH - size) - buf_offset, 0) == -1)
//  			fatal_perror("send"); //  TODO: EWOULDBLOCK
