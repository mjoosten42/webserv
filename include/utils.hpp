#pragma once

#include "stringutils.hpp"

#include <sstream>
#include <sys/poll.h>
#include <vector>

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
std::string getPollFdsAsString(InputIt first, InputIt last) {
	std::string PollFds = "{ ";
	for (; first != last; first++) {
		PollFds += toString(first->fd);
		if (first + 1 != last)
			PollFds += ",";
		PollFds += " ";
	}
	PollFds += "}";
	return PollFds;
}

void		fatal_perror(const char *msg);
void		set_fd_nonblocking(const int fd);

//  does std::transform on the entire container from beginning to end
template <class C, typename F>
void transformBeginEnd(C& container, F operation) {
	std::transform(container.begin(), container.end(), container.begin(), operation);
}

std::string getEventsAsString(short events);

void setFlag(short& events, int flag);
void unsetFlag(short& events, int flag);

template <typename T>
T stringToIntegral(const std::string& number) {
	std::stringstream ss(number);
	T				  value;

	if (number.empty())
		return T();
	ss >> value;
	return value;
}
