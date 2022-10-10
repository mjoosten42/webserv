#include "MIME.hpp"
#include "Response.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <fstream> // ifstream etc.
#include <sstream> // stringstream

#define SENDING_BUF_SIZE 4096
#define FILE_BUF_SIZE (SENDING_BUF_SIZE - 1024)

#include <sys/socket.h> // send

//  very much temporary
//  for the future: server also needs to be passed, as that may have custom 404 pages etc.
void sendFail(const int socket_fd, int code, const std::string& msg) {
	Response r;

	std::string message = "<h1>" + std::to_string(code) + " " + r.getStatusMessage(code) + "</h1>\r\n";
	message += "<p>something went wrong somewhere: <b>" + msg + "</b></p>\r\n";

	std::string responseText("HTTP/1.1 ");
	responseText += std::to_string(code) + " " + r.getStatusMessage(code) + "\r\n";
	responseText += "Content-Type: text/html\r\n";
	responseText += "Content-Length: " + std::to_string(message.length()) + "\r\n";
	responseText += "\r\n";

	responseText += message;

	if (send(socket_fd, responseText.c_str(), responseText.length(), 0) == -1)
		fatal_perror("send");
}

//  WARNING: CHUNK_MAX_LENGTH CANNOT EXCEED 0xfff as the length limit is hard coded.
//  However, there is no reason for such a high limit anyways, since browsers do not always support this.
#define CHUNK_MAX_LENGTH 1024
#define CHUNK_SENDING_SIZE (CHUNK_MAX_LENGTH + 3 + 2 * 2)

bool sendChunked(const int socket_fd, std::ifstream& infile, std::string& headers) {
	static std::stringstream ss; //  this is static so it doesn't have to be initialized every function call.

	headers += "Transfer-Encoding: chunked\r\n\r\n";

	//  send header first.
	if (send(socket_fd, headers.c_str(), headers.length(), 0) == -1)
		fatal_perror("send"); //  TODO: remove those!!

	char	 *buf = new char[CHUNK_SENDING_SIZE];
	size_t size;
	size_t bufoffset;

	while (true) {
		infile.read(buf + 5, CHUNK_MAX_LENGTH);

		if (infile.bad()) {
			//  TODO: 500, maybe exceptions?
			//  fatal_perror("readddd");
			std::cerr << "Reading infile failed!\n";
			sendFail(socket_fd, 500, "Reading file failed");
			delete[] buf;
			return false;
		}

		//  if we have reached EOF, then we finish the multichunked response with empty data.
		size = infile.gcount();
		if (size == 0) {
			size = 5;
			std::memcpy(buf, "0\r\n\r\n", size);
			if (send(socket_fd, buf, size, 0) == -1)
				fatal_perror("send");
			break;
		}

		//  add the size of the chunk, and finish the buffer with CRLF
		{
			ss.seekp(std::ios::beg);
			ss << std::hex << size;

			size_t size_str_len = ss.tellp();
			bufoffset			= 3 - size_str_len;

			std::memcpy(buf + bufoffset, ss.str().c_str(), size_str_len);
			std::memcpy(buf + bufoffset + size_str_len, "\r\n", 2);

			std::memcpy(buf + bufoffset + size_str_len + 2 + size, "\r\n", 2);
		}

		if (send(socket_fd, buf + bufoffset, CHUNK_SENDING_SIZE - (CHUNK_MAX_LENGTH - size) - bufoffset, 0) == -1)
			fatal_perror("send");
	}

	delete[] buf;
	return true;
}

//  TODO: think about architecture; how would we modularlize this?
//  Also, remove fixed size, make it dynamic so that the headers can be arbitrarily large.
bool sendSingle(const int socket_fd, std::ifstream& infile, std::string& headers) {
	char	 *buf = new char[SENDING_BUF_SIZE];
	size_t size;

	infile.read(buf, FILE_BUF_SIZE);
	if (infile.bad()) {
		//  TODO: 500, maybe exceptions?
		//  fatal_perror("readddd");
		std::cerr << "Reading infile failed!\n";
		sendFail(socket_fd, 500, "Reading file failed");
		delete[] buf;
		return false;
	}
	size = infile.gcount();
	headers += "Content-Length: " + std::to_string(size) + "\r\n\r\n"; //  TODO: REMOVE CPP11
	std::memmove(buf + headers.length(), buf, size);
	std::memcpy(buf, headers.c_str(), headers.length());

	ssize_t bytes_sent = send(socket_fd, buf, headers.length() + size, 0);
	if (bytes_sent == -1)
		fatal_perror("send");
	delete[] buf;
	return true;
}

bool transferFile(const int socket_fd, std::ifstream& infile, std::string& headers) {
	std::streampos begin = infile.tellg();
	infile.seekg(0, std::ios::end);
	std::streampos end = infile.tellg();
	infile.seekg(0, std::ios::beg);

	if (end - begin > FILE_BUF_SIZE - FILE_BUF_SIZE + 1) {
		//  send multichunked
		sendChunked(socket_fd, infile, headers);
	} else {
		//  send in single buf.
		sendSingle(socket_fd, infile, headers);
	}
	infile.close();
	return true;
}

void handleGetWithStaticFile(const int socket_fd, const std::string& filename) {
	std::ifstream infile("." + filename, std::ios::in | std::ios::binary); //  TODO: remove dot

	if (!infile.is_open()) {
		//  TODO: send 401, 404 or 500
		sendFail(socket_fd, 404, "Opening file failed whoops");
		return;
	}

	std::string headers = std::string("HTTP/1.1 200 OK\r\n"
									  "Connection: Keep-Alive\r\n"
									  "Content-Type: ");
	headers += MIME::fromFileName(filename) + "\r\n";

	transferFile(socket_fd, infile, headers);
}
