#include "stringutils.hpp"

#include "utils.hpp"

#include <algorithm>

//  converts an ASCII string to lowercase
void strToLower(std::string& str) {
	transformBeginEnd(str, ::tolower);
}

//  converts an ASCII string to uppercase
void strToUpper(std::string& str) {
	transformBeginEnd(str, ::toupper);
}

/* sets a hex string to s based on single byte b.
 * At the first char we merely need to shift it as the
 * result of shifting itself removes the other nibble. */
static void m_itoa_hex_byte(char *s, const unsigned char b) {
	const char *hex = "0123456789abcdef";

	s[0]			= hex[b >> 4];
	s[1]			= hex[b & 0x0f];
}

//  copies string s with at most len characters, allocates on the heap.
char *ft_strndup(char *s, size_t len) {
	char *s2 = new char[len];
	std::memcpy(s2, s, len);
	s2[len] = '\0';
	return s2;
}

/* does itoa for a size_t, but in hexadecimal. */
char *ft_itoa_hex_size_t(size_t nbr) {
	char   s[sizeof(size_t) * 2];
	size_t i;
	size_t j;

	i = 0;
	j = (sizeof(size_t) - 1) * 2;
	while (i < sizeof(size_t)) {
		m_itoa_hex_byte((s + j), (reinterpret_cast<unsigned char *>(&nbr))[i]);
		i++;
		j -= 2;
	}

	//  remove leading zeroes
	i = 0;
	while (i < sizeof(size_t) * 2 - 1 && s[i] == '0')
		i++;
	return (ft_strndup(s + i, sizeof(size_t) * 2 - i));
}
