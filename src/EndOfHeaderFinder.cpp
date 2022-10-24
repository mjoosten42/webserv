#include "EndOfHeaderFinder.hpp"

EndOfHeaderFinder::EndOfHeaderFinder(): m_prevNL(false) {}

// Finds an end of a HTTP1.1 header. An header end is when there are two new lines, either \r\n or just \n.
// It keeps the state of when you called it the previous time. For example, if you previously ended with
// \n, it will keep that in mind. When it finds the end, the state is reset.
size_t EndOfHeaderFinder::find(const std::string& str) {

	for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
		if (*it == '\n') {
			if (m_prevNL) {
				reset();
				return ++it - str.begin();
			}
			m_prevNL = true;
		} else if (*it != '\r') {
			m_prevNL = false;
		}
	}
	return std::string::npos;
}

// resets the state.
void EndOfHeaderFinder::reset() {
	m_prevNL = false;
}
