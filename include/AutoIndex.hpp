#pragma once

#include <dirent.h> // DIR *, opendir, closedir etc.
#include <string>
#include <vector>

struct Entry {
		std::string		   name;
		std::vector<Entry> subdir;

		std::string toString() const;
};

std::string		   basename(const std::string		&path);
std::string		   autoIndexHtml(std::string absolute_dir_path);
std::vector<Entry> recursivePathCount(const std::string directory);
