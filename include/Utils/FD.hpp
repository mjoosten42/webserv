#pragma once

#include <map>

class FD {
		static std::map<int, int> m_references; // Map fd to the amount of copies it has

	public:
		FD();
		FD(int fd);
		FD(const FD &other);
		~FD();

		FD &operator=(const FD &rhs);

		operator int() const; // Implicit conversion

	private:
		void increase();
		void decrease();

	private:
		int m_fd;
};
