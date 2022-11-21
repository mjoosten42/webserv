#pragma once

#include <sstream>
#include <string>
#include <vector>

void strToLower(std::string &str);
void strToUpper(std::string &str);

void trimLeadingWhiteSpace(std::string &str);
void trimTrailingWhiteSpace(std::string &str);

std::vector<std::string> stringSplit(const std::string &s);

std::string getEventsAsString(short events);

template <typename T>
std::string toString(const T &value) {
	std::stringstream ss;

	ss << value;
	return ss.str();
}

template <typename T>
std::string toHex(const T &value) {
	std::stringstream ss;

	ss << std::hex << value;
	return ss.str();
}
