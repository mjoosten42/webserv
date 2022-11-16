#include "FD.hpp"

#include "logger.hpp"
#include "syscalls.hpp"

std::map<int, int> FD::m_references = { { -1, 1 } }; // Add a copy of -1 so it won't be closed while running

FD::FD(): m_fd(-1) {
	increase();
}

FD::FD(int fd): m_fd(fd) {
	increase();
}

FD::FD(const FD& other): m_fd(other.m_fd) {
	increase();
}

FD::~FD() {
	decrease();
}

FD& FD::operator=(const FD& rhs) {
	decrease();
	m_fd = rhs.m_fd;
	increase();
	return *this;
}

FD::operator int() const {
	return m_fd;
}

void FD::increase() {
	m_references[m_fd]++;
}

void FD::decrease() {
	m_references[m_fd]--;
	if (m_references[m_fd] == 0)
		WS::close(m_fd);
}
