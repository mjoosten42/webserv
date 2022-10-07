#include "utils.hpp"

#include <fcntl.h> // fcntl
#include <map>
#include <stdio.h>	// perror
#include <stdlib.h> // exit
#include <string>
#include <vector>

//  perrors and exits.
void fatal_perror(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

//  sets file descriptor fd to nonblocking mode
void set_fd_nonblocking(const int fd) {
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		fatal_perror("fcntl");
}

void printPollFds(const std::vector<pollfd>& vector) {
	std::cout << "Fds: { ";
	for (uint i = 0; i < vector.size(); i++) {
		std::cout << vector[i].fd;
		if (i + 1 < vector.size())
			std::cout << ", ";
	}
	std::cout << " }\n";
}

void printStringMap(const std::map<std::string, std::string>& map) {
	std::map<std::string, std::string>::const_iterator it = map.begin();
	std::cout << "Map: {\n";
	for (; it != map.end(); ++it)
		std::cout << "  { " << it->first << ", " << it->second << " }\n";
	std::cout << "}\n";
}

//TODO: this needs to be an enum. Or maybe even a cpp string?
void printMethod(int method) {
	std::cout << "Method: ";
	if (method == 0)
		std::cout << "GET\n";
	if (method == 1)
		std::cout << "POST\n";
	if (method == 2)
		std::cout << "DELETE\n";
}

#include <fstream> // ifstream etc.

#define SENDING_BUF_SIZE 4096
#define FILE_BUF_SIZE (SENDING_BUF_SIZE - 1024)

#include <sys/socket.h> // send

// TODO: think about architecture; how would we modularlize this?
// 
bool sendSingle(const int socket_fd, std::ifstream& infile, std::string& headers)
{
	char buf[SENDING_BUF_SIZE];
	size_t	size;

	infile.read(buf, FILE_BUF_SIZE);
	if (infile.bad())
	{
		// TODO: 500, maybe exceptions?
		fatal_perror("readddd");
		return false;
	}
	size = infile.gcount();
	headers += "Content-Length: " + std::to_string(size) + "\r\n\r\n";// tODO: REMOVE CPP11
	std::memmove(buf + headers.length(), buf, size);
	std::memcpy(buf, headers.c_str(), headers.length());
	std::memcpy(buf + headers.length() + size, "\r\n", 2);
	
	// TODO: fix bug here

	print(buf);

	std::cout << "yeet\n";

	ssize_t bytes_sent = send(socket_fd, buf, headers.length() + size + 2, 0);
	if (bytes_sent == -1)
	{
		fatal_perror("send");
	}
	std::cout << "yeet\n";
	return true;
}

// TODO: move
bool transferFile(const int socket_fd, std::ifstream& infile, std::string& headers)
{
	std::streampos begin = infile.tellg();
	infile.seekg(0, std::ios::end);
	std::streampos end = infile.tellg();
	infile.seekg(0, std::ios::beg);

	if (end - begin > FILE_BUF_SIZE)
	{
		// send multichunked
		fatal_perror("NOT IMPLEMENTED");
	}
	{
		// send in single buf.
		sendSingle(socket_fd, infile, headers);
	}
	infile.close();
	return true;
}

void handleGetWithStaticFile(const int socket_fd, const std::string& filename)
{
	std::ifstream infile(filename, std::ios::in | std::ios::binary);

	if (!infile.is_open())
	{
		// TODO: send 404 or 500
		return ;
	}

	std::string headers = std::string("HTTP/1.1 200 OK\r\n"
					"Connection: Keep-Alive\r\n"
					"Content-Type: text/plain\r\n");
	transferFile(socket_fd, infile, headers);
}
