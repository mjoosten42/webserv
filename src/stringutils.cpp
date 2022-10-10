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
