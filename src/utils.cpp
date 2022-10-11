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

std::string getStringMapAsString(const std::map<std::string, std::string>& map) {
	std::map<std::string, std::string>::const_iterator it = map.begin();
	std::string										   strings;

	for (; it != map.end(); ++it)
		strings += "  " + it->first + ": " + it->second + "\n";
	return strings;
}
