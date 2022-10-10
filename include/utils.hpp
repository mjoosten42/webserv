#pragma once

#include <iostream>
#include <map>
#include <sys/poll.h>
#include <vector>

//  TODO: move to defines.hpp?
#define CRLF "\r\n"

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
		std::cout << "{ " << it->first << " " << it->second << " } ";
		next = it;
		next++;
		if (next != map.end())
			std::cout << ", ";
	}
	std::cout << " }\n";
}

//  a binary search template. key is the ky you want to search. The entries is an array of structs with a key and value.
//  Size is the length of the array. The compare function should substract b from a, ex. strcmp.
template <class R, class T, class A>
R binarySearchKeyValue(T key, A entries, const int size) {
	int pivot = size / 2;
	int min	  = 0;
	int max	  = size - 1;

	while (min <= max) {
		if (key < entries[pivot].key)
			max = pivot - 1;

		else if (entries[pivot].key < key)
			min = pivot + 1;

		else //  if (delta == 0) is implied
			return entries[pivot].value;
		pivot = (min + max) / 2;
	}
	return nullptr;
}

template <typename InputIt>
void printFds(InputIt first, InputIt last) {
	std::cout << "{ ";
	for (; first != last; first++) {
		std::cout << first->fd;
		if (first + 1 < last)
			std::cout << ", ";
	}
	std::cout << " }\n";
}

void printStringMap(const std::map<std::string, std::string>& map);
void printMethod(int method);
void fatal_perror(const char *msg);
void set_fd_nonblocking(const int fd);

//  does std::transform on the entire container from beginning to end
template <class T, typename F>
void transformBeginEnd(T& container, F operation) {
	std::transform(container.begin(), container.end(), container.begin(), operation);
}
