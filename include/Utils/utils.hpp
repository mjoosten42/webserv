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

void setFlag(short& events, int flag);
void unsetFlag(short& events, int flag);

template <typename T>
T stringToIntegral(const std::string& number) {
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

template <typename T, typename F>
void overwriteIfSpecified(const std::string& search, T& field, t_block_directive *constructor_specs, F fun) {
	std::string value = constructor_specs->fetch_simple(search);

	if (!value.empty())
		field = fun(value);
}
