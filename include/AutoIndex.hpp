#pragma once

#include <dirent.h> // DIR *, opendir, closedir etc.
#include <string>
#include <vector>

struct Entry {
		std::string		   name;
		std::vector<Entry> subdir;

		std::string toString(const std::string& path = "") const;
};

std::vector<Entry> recursivePathCount(const std::string directory);

std::string basename(const std::string& path);
