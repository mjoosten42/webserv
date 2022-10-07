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

//  converts an ASCII string to lowercase
void strToLower(std::string& str) {
	transformBeginEnd(str, ::tolower);
}

//  converts an ASCII string to uppercase
void strToUpper(std::string& str) {
	transformBeginEnd(str, ::toupper);
}
