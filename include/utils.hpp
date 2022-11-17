#pragma once

#include "stringutils.hpp"

#include <algorithm> // transform
#include <sstream>
#include <sys/poll.h>
#include <vector>

bool operator<(const pollfd& lhs, const pollfd& rhs);
std::ostream& operator<<(std::ostream& os, const pollfd& pfd);

struct KeyValue {
		const char *key;
		const char *value;
};

// A binary search template. Key is the key you want to search.
// The entries is an array of structs with a key and value.
// Size is the length of the array.
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

void fatal_perror(const char *msg);
void set_fd_nonblocking(const int fd);

// does std::transform on the entire container from beginning to end
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

size_t winSize();
off_t  fileSize(int fd);
size_t match(const std::string first, const std::string& second);
bool   isDir(const std::string	&path);
bool   isGood(int status);

std::string getExtension(const std::string& filename);
std::string basename(const std::string& path);
std::string addressToString(int address);

template <typename InputIt>
std::string rangeToString(InputIt first, InputIt last) {
	std::string ret = "{ ";
	for (; first != last; first++) {
		ret += toString(*first);
		if (++first != last)
			ret += ",";
		first--;
		ret += " ";
	}
	return ret + "}";
}
