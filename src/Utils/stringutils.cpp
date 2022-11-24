#include "stringutils.hpp"

#include "defines.hpp"
#include "utils.hpp"

// converts an ASCII string to lowercase
void strToLower(std::string &str) {
	transformBeginEnd(str, ::tolower);
}

// converts an ASCII string to uppercase
void strToUpper(std::string &str) {
	transformBeginEnd(str, ::toupper);
}

// trims str leading and trailing characters from set
void trim(std::string &str, const std::string &set) {
	size_t start = str.find_first_not_of(set);
	size_t end	 = str.find_last_not_of(set) + 1;

	if (end != std::string::npos)
		str.resize(end);

	if (start != std::string::npos)
		str.erase(0, start);
}

// splits a string on whitespace chars.
std::vector<std::string> stringSplit(const std::string &s) {
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

std::string getEventsAsString(short revents) {
	std::string events;

	if (revents & POLLIN)
		events += "IN";
	if (revents & POLLOUT) {
		if (!events.empty())
			events += " | ";
		events += "OUT";
	}
	if (revents & POLLHUP) {
		if (!events.empty())
			events += " | ";
		events += "HUP";
	}
	if (revents & POLLNVAL) {
		if (!events.empty())
			events += " | ";
		events += "NVAL";
	}
	return "{ " + events + " }";
}

#define HTTP_SEPERATORS ("()<>@,;:\\\"/[]?={} \t")

// returns true if the string complies to a HTTP token as specified in RFC2616.
// basically, a word.
bool isHTTPToken(const std::string& str)
{
	for (auto c : str) {
		if (!isascii(c) || std::string(HTTP_SEPERATORS).find(c) != std::string::npos) {
			return false;
		}
	}
	return true;
}
