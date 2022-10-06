#pragma once

#include <iostream>
#include <map>
#include <sys/poll.h>
#include <vector>

template <typename T>
void print(const T& value) {
	std::cout << value << "\n";
}

template <typename T, typename U>
void printMap(const std::map<T, U>& map) {
	typename std::map<T, U>::const_iterator it;
	typename std::map<T, U>::const_iterator next;
	std::cout << "Map: { ";
	for (it = map.begin(); it != map.end(); ++it) {
		std::cout << "{ " << it->first << ", " << it->second << " } ";
		next = it;
		next++;
		if (next != map.end())
			std::cout << ", ";
	}
	std::cout << " }\n";
}

void printPollFds(const std::vector<pollfd>& vector);
void fatal_perror(const char *msg);
void set_fd_nonblocking(const int fd);
