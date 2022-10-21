#pragma once

#include <sstream>
#include <string>

void					 strToLower(std::string					   &str);
void					 strToUpper(std::string					   &str);
std::string				 trimLeadingWhiteSpace(const std::string			 &s);
std::string				 trimTrailingWhiteSpace(const std::string			  &s);
std::vector<std::string> stringSplit(const std::string& s);

//  TODO: performance?
template <typename T>
std::string toString(const T& value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template <typename T>
std::string toHex(const T& value) {
	std::stringstream ss;

	ss.seekp(std::ios::beg);
	ss << std::hex << value;
	return ss.str();
}
