#include "buffer.hpp"

#include <limits.h> // PATH_MAX

// Global buffer
char buf[BUFFER_SIZE] = { 0 };

// Used for realpath
char path[PATH_MAX + 1] = { 0 };
