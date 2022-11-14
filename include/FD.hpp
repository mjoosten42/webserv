#pragma once

class FD {
	public:
		FD();
		FD(int fd);
		FD(const FD& other);
		~FD();

		FD& operator=(const FD& rhs);

		operator int() const; // Implicit conversion

	private:
		void increase();
		void decrease();

	private:
		int	 m_fd;
		int *m_copies;
};
