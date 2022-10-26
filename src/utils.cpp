#include "utils.hpp"

#include <fcntl.h> // fcntl
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

std::string getEventsAsString(short revents) {
	std::string events;

	if (revents & POLLIN)
		events += "IN";
	if (revents & POLLOUT) {
		if (!events.empty())
			events += " | ";
		events += "OUT";
	}
	if (revents & POLLHUP) {
		if (!events.empty())
			events += " | ";
		events += "HUP";
	}
	if (revents & POLLNVAL) {
		if (!events.empty())
			events += " | ";
		events += "NVAL";
	}
	return events;
}

void setFlag(short& events, int flag) {
	events |= flag;
}

void unsetFlag(short& events, int flag) {
	events &= ~flag;
}
