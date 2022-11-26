#pragma once

#include "ConfigParser.hpp"
#include "IO.hpp"
#include "stringutils.hpp"

#include <algorithm> // transform
#include <sstream>
#include <stdexcept>
#include <sys/poll.h>
#include <vector>

struct KeyValue {
		const char *key;
		const char *value;
};

// A binary search template. Key is the key you want to search.
// The entries is an array of structs with a key and value.
// Size is the length of the array.
template <class Key, class Value>
const char *binarySearchKeyValue(Key key, Value entries[], size_t size) {
	size_t pivot = size / 2;
	size_t min	 = 0;
	size_t max	 = size - 1;

	if (key >= entries[min].key && key <= entries[max].key) {
		while (min <= max) {
			if (key < entries[pivot].key)
				max = pivot - 1;
			else if (key > entries[pivot].key)
				min = pivot + 1;
			else // key == entries[pivot].key is implied
				return entries[pivot].value;
			pivot = (min + max) / 2;
		}
	}
	return NULL;
}

void fatal_perror(const char *msg);
void set_fd_nonblocking(int fd);

// does std::transform on the entire container from beginning to end
template <class C, typename F>
void transformBeginEnd(C &container, F operation) {
	std::transform(container.begin(), container.end(), container.begin(), operation);
}

void setFlag(short &events, int flag);
void unsetFlag(short &events, int flag);

template <typename T>
T stringToIntegral(const std::string &number) {
	std::stringstream ss(number);
	T				  value;

	if (number.empty())
		return T();
	ss >> value;
	if (ss.fail())
		throw std::overflow_error("Number too big: " + toString(number));
	return value;
}

size_t winSize();
size_t match(const std::string first, const std::string &second);

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
