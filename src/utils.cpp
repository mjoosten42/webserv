#include "utils.hpp"
#include "logger.hpp"

#include <fcntl.h>	// fcntl
#include <stdio.h>	// perror
#include <stdlib.h> // exit
#include <string>
#include <vector>

#include <sys/stat.h> // stat

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
		events += "NVAL"; // TODO: This is returned when client navigates to directory without trailing slash.
	}
	return events;
}

void setFlag(short& events, int flag) {
	events |= flag;
}

void unsetFlag(short& events, int flag) {
	events &= ~flag;
}

// returns true if path is a directory. False when not or stat errors.
bool isDirectory(const char *path) {
	struct stat s;
	if (stat(path, &s) == -1) {
		LOG_ERR("stat: " <<  strerror(errno));
		return false;
	}
	return static_cast<bool>(S_ISDIR(s.st_mode));
}
