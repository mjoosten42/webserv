#pragma once

#include <iostream>
#include <sys/poll.h>
#include <vector>

template <typename T>
void print(const T& value) {
	std::cout << value << "\n";
}

void printPollFds(const std::vector<pollfd>& vector);
void fatal_perror(const char *msg);
void set_fd_nonblocking(const int fd);
