#include "stringutils.hpp"

#include "utils.hpp"

//  converts an ASCII string to lowercase
void strToLower(std::string& str) {
	transformBeginEnd(str, ::tolower);
}

//  converts an ASCII string to uppercase
void strToUpper(std::string& str) {
	transformBeginEnd(str, ::toupper);
}

std::string trimLeadingWhiteSpace(const std::string& s) {
	const std::string whitespaceChars = " \n\r\t\f\v";
	size_t			  start			  = s.find_first_not_of(whitespaceChars);
	std::string		  ret			  = "";
	if (start != std::string::npos)
		ret = s.substr(start);
	return (ret);
}
