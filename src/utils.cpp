#include "utils.hpp"

#include <dirent.h> //recursiveFileCount
#include <fcntl.h>	// fcntl
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

#include "logger.hpp"

// Returns the number of files and directories in the specified directory recursively.
// Stores the names of all files and directories in a string vector it is passed as a param.
// These stored names have leading tabs to indicate directory structure.
unsigned int
	recursiveFileCount(const std::string directory, std::vector<std::string>& file_structure, std::string tabulation) {
	DIR			*derp; // DirEctoRy Pointer
	unsigned int ret = 0;

	derp = opendir(directory.c_str());
	if (derp == NULL)
		return (ret);
	struct dirent *contents;
	while ((contents = readdir(derp)) != NULL) {
		std::string name = contents->d_name;
		if (name != "." && name != "..") {
			file_structure.push_back(tabulation + name);
			ret++;
			std::string next_dir = directory + "/" + name;
			ret					 = ret + recursiveFileCount(next_dir, file_structure, "\t" + tabulation);
		}
	}
	closedir(derp);
	return (ret);
}
