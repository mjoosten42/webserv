#pragma once

#include <string>

class EndOfHeaderFinder {
	public:
		EndOfHeaderFinder();

		size_t find(const std::string& str);
		void   reset();

	private:
		bool m_prevNL; // previous char was a newline
};
