#pragma once

#include <iostream>
#include <map>
#include <sys/poll.h>
#include <vector>

//  Terminal colors
#define RED "\033[0;31m"
#define DEFAULT "\033[0m"

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

//  A binary search template. Key is the key you want to search.
//  The entries is an array of structs with a key and value.
//  Size is the length of the array.
template <class Key, class Value>
const char *binarySearchKeyValue(Key key, Value entries[], int size) {
	int pivot = size / 2;
	int min	  = 0;
	int max	  = size - 1;

	while (min <= max) {
		if (key < entries[pivot].key)
			max = pivot - 1;

		else if (entries[pivot].key < key)
			min = pivot + 1;

		else
			return entries[pivot].value;
		pivot = (min + max) / 2;
	}
	return NULL;
}

template <typename InputIt>
std::string printFds(InputIt first, InputIt last) {
	std::string Fds;
	Fds += "{ ";
	for (; first != last; first++) {
		Fds += first->fd;
		if (first + 1 < last)
			Fds += ", ";
	}
	Fds += " }\n";
	return Fds;
}

std::string getStringMapAsString(const std::map<std::string, std::string>& map);
void		fatal_perror(const char *msg);
void		set_fd_nonblocking(const int fd);

//  does std::transform on the entire container from beginning to end
template <class T, typename F>
void transformBeginEnd(T& container, F operation) {
	std::transform(container.begin(), container.end(), container.begin(), operation);
}
