#pragma once

#include <string>
#include <unistd.h> // off_t

off_t		fileSize(int fd);
bool		isDir(const std::string &path);
std::string extension(const std::string &filename);
std::string basename(const std::string &path);
