#include "stringutils.hpp"

#include "defines.hpp"
#include "utils.hpp"

// converts an ASCII string to lowercase
void strToLower(std::string& str) {
	transformBeginEnd(str, ::tolower);
}

// converts an ASCII string to uppercase
void strToUpper(std::string& str) {
	transformBeginEnd(str, ::toupper);
}

std::string trimLeadingWhiteSpace(const std::string& s) {
	size_t		start = s.find_first_not_of(IFS);
	std::string ret	  = "";
	if (start != std::string::npos)
		ret = s.substr(start);
	return (ret);
}

unsigned int countAndTrimLeadingWhiteSpace(std::string& s) {
	size_t start = s.find_first_not_of(IFS);
	if (start != std::string::npos) {
		s = s.substr(start);
		return (start);
	}
	return (0);
}

std::string trimTrailingWhiteSpace(const std::string& s) {
	size_t		end = s.find_last_not_of(IFS);
	std::string ret;

	if (end != std::string::npos)
		ret = s.substr(0, end + 1);
	return (ret);
}

// splits a string on whitespace chars.
std::vector<std::string> stringSplit(const std::string& s) {
	std::vector<std::string> ret;
	size_t					 begin = 0;
	size_t					 end   = 0;

	while (end != std::string::npos) {
		begin = s.find_first_not_of(IFS, end);
		end	  = s.find_first_of(IFS, begin);
		if (begin != std::string::npos)
			ret.push_back(s.substr(begin, end - begin));
	}
	return (ret);
}
