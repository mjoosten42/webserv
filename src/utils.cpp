#include "utils.hpp"

#include "logger.hpp"

#include <fcntl.h>	// fcntl
#include <stdio.h>	// perror
#include <stdlib.h> // exit
#include <string>
#include <sys/ioctl.h> // ioctl
#include <sys/stat.h>  // stat
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

// gets the width of the terminal window
size_t winSize() {
	struct winsize w;
	ioctl(1, TIOCGWINSZ, &w);
	return w.ws_col;
}

size_t findNewline(const std::string str, size_t begin) {
	size_t pos = str.find("\r\n", begin);
	if (pos != std::string::npos)
		return pos;
	return str.find("\n", begin);
}

std::string getRealPath(const std::string& str) {
	static char resolved_path[PATH_MAX + 1] = { 0 };
	char	   *ret							= realpath(str.c_str(), resolved_path);

	if (!ret) {
		LOG_ERR("realpath: " << str << ": " << strerror(errno));
		return "";
	}
	return resolved_path;
}

off_t fileSize(int fd) {
	off_t size = lseek(fd, 0, SEEK_END);

	if (size == -1) {
		LOG_ERR("lseek: " << strerror(errno));
		size = std::numeric_limits<off_t>().max();
	}
	lseek(fd, 0, SEEK_SET); // set back to start

	return size;
}

size_t match(const std::string first, const std::string& second) {
	size_t len = 0;

	while (len < first.length() && len < second.length()) {
		if (first[len] != second[len])
			break;
		len++;
	}
	return len;
}

std::string getExtension(const std::string& filename) {
	size_t dot = filename.find_last_of('.');

	if (dot != filename.npos)
		return filename.substr(dot + 1);
	return "";
}
