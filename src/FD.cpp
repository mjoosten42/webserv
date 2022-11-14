#include "FD.hpp"

#include "syscalls.hpp"

FD::FD(): m_copies(NULL) {}

FD::FD(int fd): m_fd(fd), m_copies(new int(1)) {}

FD::FD(const FD& other): m_fd(other.m_fd), m_copies(other.m_copies) {
	increase();
}

FD::~FD() {
	decrease();
}

FD& FD::operator=(const FD& rhs) {
	decrease();
	m_fd	 = rhs.m_fd;
	m_copies = rhs.m_copies;
	increase();
	return *this;
}

FD::operator int() const {
	return m_fd;
}

void FD::increase() {
	if (m_copies)
		(*m_copies)++;
}

void FD::decrease() {
	if (m_copies) {
		(*m_copies)--;
		if (!(*m_copies)) {
			delete m_copies;
			WS::close(m_fd);
		}
	}
}
