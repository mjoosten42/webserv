#pragma once

#include <sstream>
#include <string>

void		strToLower(std::string		 &str);
void		strToUpper(std::string		 &str);
std::string trimLeadingWhiteSpace(const std::string& s);
std::string trimTrailingWhiteSpace(const std::string& s);
std::vector<std::string> stringSplit(const std::string& s);

//  TODO: performance?
template <typename T>
std::string toString(const T& t) {
	std::stringstream ss;
	ss << t;
	return ss.str();
}

//  TODO: stringstream stuff with to_string?
//  http://cplusplus.bordoon.com/speeding_up_string_conversions.html
