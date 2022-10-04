#pragma once
#include <iostream>
#include <sys/poll.h>
#include <vector>

template <typename T>
void print(const T& value) {
	std::cout << value << "\n";
}

template <typename Allocator>
void print(const std::vector<pollfd, Allocator>& vector) {
	std::cout << "{ ";
	for (uint i = 0; i < vector.size(); i++) {
		std::cout << vector[i].fd;
		if (i + 1 < vector.size())
			std::cout << ", ";
	}
	std::cout << " }\n";
}

void fatal_perror(const char *msg);

void set_fd_nonblocking(const int fd);
