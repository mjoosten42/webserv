#include "file.hpp"

#include "logger.hpp"

#include <dirent.h> // opendir, closedir, DIR
#include <unistd.h> // lseek, off_t

bool isDir(const std::string &path) {
	DIR *dir = opendir(path.c_str());

	if (!dir)
		return false;
	if (closedir(dir) == -1)
		perror("closedir");
	return true;
}

off_t fileSize(int fd) {
	off_t size = lseek(fd, 0, SEEK_END);

	if (size == -1) {
		LOG_ERR("lseek: " << strerror(errno));
		throw 500;
	}
	lseek(fd, 0, SEEK_SET); // set back to start

	return size;
}

std::string extension(const std::string &filename) {
	size_t dot = filename.find_last_of('.');

	if (dot != filename.npos)
		return filename.substr(dot + 1);
	return "";
}

std::string basename(const std::string &path) {
	std::string base = path;

	while (!base.empty() && base.back() == '/')
		base.pop_back();

	size_t pos = base.find_last_of("/");

	if (pos != std::string::npos)
		return base.substr(pos + 1);
	return path;
}
