#include "utils.hpp"

#include "logger.hpp"

#include <arpa/inet.h> // ntohl
#include <dirent.h>	   // opendir
#include <fcntl.h>	   // fcntl
#include <stdio.h>	   // perror
#include <stdlib.h>	   // exit
#include <string>
#include <sys/ioctl.h> // ioctl
#include <unistd.h>	   // lseek
#include <vector>

// perrors and exits.
void fatal_perror(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

// sets file descriptor fd to nonblocking mode
void set_fd_nonblocking(const int fd) {
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		fatal_perror("fcntl");
}

void setFlag(short &events, int flag) {
	events |= flag;
}

void unsetFlag(short &events, int flag) {
	events &= ~flag;
}

// gets the width of the terminal window
size_t winSize() {
	struct winsize w = { 0, 0, 0, 0 };
	if (isatty(STDOUT_FILENO))
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
}

size_t match(const std::string first, const std::string &second) {
	size_t len = 0;

	while (len < first.length() && len < second.length()) {
		if (first[len] != second[len])
			break;
		len++;
	}
	return len;
}

std::string addressToString(int address) {
	std::string str;

	for (int i = 24; i >= 0; i -= 8) {
		str += toString((ntohl(address) >> i) & 0xFF);
		if (i)
			str += ".";
	}
	return str;
}
