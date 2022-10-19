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

std::string trimTrailingWhiteSpace(const std::string& s) {
	const std::string whitespaceChars = " \n\r\t\f\v";
	size_t			  end			  = s.find_last_not_of(whitespaceChars);
	std::string		  ret			  = "";
	if (end != std::string::npos)
		ret = s.substr(0, end + 1);
	return (ret);
}

std::vector<std::string> stringSplit(const std::string& s)
{
	std::vector<std::string> ret;
	const std::string whitespaceChars = " \n\r\t\f\v";

	size_t begin = 0;
	size_t end = 0;
	while (end != std::string::npos)
	{
		begin = s.find_first_not_of(whitespaceChars, end);
		end = s.find_first_of(whitespaceChars, begin);
		if (begin != std::string::npos)
			ret.push_back(s.substr(begin, end - begin));
	}
	return(ret);
}
