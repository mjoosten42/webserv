#include "utils.hpp"

#include <stdio.h> // perror
#include <stdlib.h> // exit

// perrors and exits.
void	fatal_perror(const char *msg)
{
	perror(msg);
	exit(1);
}
