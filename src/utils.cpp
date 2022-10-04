#include "utils.hpp"

#include <stdio.h> // perror
#include <stdlib.h> // exit
#include <fcntl.h> // fcntl

// perrors and exits.
void	fatal_perror(const char *msg)
{
	perror(msg);
	exit(1);
}

// sets file descriptor fd to nonblocking mode
void	set_fd_nonblocking(const int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		fatal_perror("fcntl");
	}
}
