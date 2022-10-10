#pragma once

#include <iostream>
#include <unistd.h>

class shared_fd {
	public:
		shared_fd();
		shared_fd(int *p);
		shared_fd(const shared_fd& other);
		~shared_fd();
		shared_fd& operator=(const shared_fd& other);
		int		   operator*() const;
				   operator int() const;

	private:
		int *ptr;
		int *copies;
};

shared_fd make_shared(int fd);
