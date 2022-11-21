#include "utils.hpp"

#include "buffer.hpp" // path
#include "logger.hpp"

#include <arpa/inet.h> // ntohl
#include <dirent.h>	   // opendir
#include <fcntl.h>	   // fcntl
#include <stdio.h>	   // perror
#include <stdlib.h>	   // exit
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

void setFlag(short& events, int flag) {
	events |= flag;
}

void unsetFlag(short& events, int flag) {
	events &= ~flag;
}

// gets the width of the terminal window
size_t winSize() {
	struct winsize w = { 0, 0, 0, 0 };
	if (isatty(STDOUT_FILENO))
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
}

off_t fileSize(int fd) {
	off_t size = lseek(fd, 0, SEEK_END);

	if (size == -1) {
		LOG_ERR("lseek: " << strerror(errno));
		throw 500;
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

bool isDir(const std::string& path) {
	DIR *dir = opendir(path.c_str());

	if (!dir)
		return false;
	if (closedir(dir) == -1)
		perror("closedir");
	return true;
}

bool isGood(int status) {
	return status < 400;
}

std::string basename(const std::string& path) {
	std::string base = path;

	if (base.back() == '/')
		base.pop_back();

	size_t pos = base.find_last_of("/");

	if (pos != std::string::npos)
		return base.substr(pos);
	return path;
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
