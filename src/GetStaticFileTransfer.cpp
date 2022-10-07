#include "utils.hpp"

#include <fstream> // ifstream etc.

#define SENDING_BUF_SIZE 4096
#define FILE_BUF_SIZE (SENDING_BUF_SIZE - 1024)

#include <sys/socket.h> // send

//  TODO: think about architecture; how would we modularlize this?
//  TODO: remove fixed buffer, use dynamic memory allocation instead,
//  also for the headers so that they can be arbitrarily large.
bool sendSingle(const int socket_fd, std::ifstream& infile, std::string& headers) {
	char   buf[SENDING_BUF_SIZE];
	size_t size;

	infile.read(buf, FILE_BUF_SIZE);
	if (infile.bad()) {
		//  TODO: 500, maybe exceptions?
		fatal_perror("readddd");
		return false;
	}
	size = infile.gcount();
	headers += "Content-Length: " + std::to_string(size) + "\r\n\r\n"; //  TODO: REMOVE CPP11
	std::memmove(buf + headers.length(), buf, size);
	std::memcpy(buf, headers.c_str(), headers.length());
	std::memcpy(buf + headers.length() + size, "\r\n", 2);

	ssize_t bytes_sent = send(socket_fd, buf, headers.length() + size + 2, 0);
	if (bytes_sent == -1)
		fatal_perror("send");
	return true;
}

bool transferFile(const int socket_fd, std::ifstream& infile, std::string& headers) {
	std::streampos begin = infile.tellg();
	infile.seekg(0, std::ios::end);
	std::streampos end = infile.tellg();
	infile.seekg(0, std::ios::beg);

	if (end - begin > FILE_BUF_SIZE) {
		//  send multichunked
		fatal_perror("NOT IMPLEMENTED");
	}
	{
		//  send in single buf.
		sendSingle(socket_fd, infile, headers);
	}
	infile.close();
	return true;
}

void handleGetWithStaticFile(const int socket_fd, const std::string& filename) {
	std::ifstream infile(filename, std::ios::in | std::ios::binary);

	if (!infile.is_open()) {
		//  TODO: send 404 or 500
		return;
	}

	std::string headers = std::string("HTTP/1.1 200 OK\r\n"
									  "Connection: Keep-Alive\r\n"
									  "Content-Type: text/html\r\n");
	transferFile(socket_fd, infile, headers);
}
