#include "shared_fd.hpp"

shared_fd::shared_fd(): ptr(NULL), copies(NULL) {}

shared_fd::shared_fd(int *p): ptr(p), copies(new int(1)) {}

shared_fd::shared_fd(const shared_fd& other): ptr(other.ptr), copies(other.copies) {
	(*copies)++;
}

shared_fd::~shared_fd() {
	if (!ptr)
		return;
	(*copies)--;
	if (*copies == 0) {
		close(*ptr);
		delete ptr;
		delete copies;
	}
}

shared_fd& shared_fd::operator=(const shared_fd& other) {
	this->~shared_fd();
	ptr	   = other.ptr;
	copies = other.copies;
	(*copies)++;
	return *this;
}

//  int conversion
shared_fd::operator int() const {
	return *ptr;
}

shared_fd make_shared(int fd) {
	return new int(fd);
}
