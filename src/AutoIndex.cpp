#include "AutoIndex.hpp"

#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

std::vector<Entry> recursivePathCount(const std::string directory) {
	std::vector<Entry> paths;
	DIR				  *derp; // DirEctoRy Pointer
	struct dirent	  *contents;
	Entry			   entry;
	std::string		   name;

	derp = opendir(directory.c_str());
	if (derp == NULL)
		return (paths);
	while ((contents = readdir(derp)) != NULL) {
		name = contents->d_name;
		if (*(name.begin()) == '.') // Skip current, up and hidden
			continue;
		entry.name = name;
		if (contents->d_type == DT_DIR) {
			entry.name += "/";
			entry.subdir = recursivePathCount(directory + entry.name);
		}
		paths.push_back(entry);
		entry.subdir.clear();
	}
	closedir(derp);
	return paths;
}

std::string Entry::toString(const std::string& path) const {
	std::string ret = "<li>\n";

	ret += "<a href=\"" + path + name + "\">" + basename(name) + "</a>\n";
	if (!subdir.empty()) {
		ret += "<ul>\n";
		for (size_t i = 0; i < subdir.size(); i++)
			ret += subdir[i].toString(path + name);
		ret += "</ul>\n";
	}
	return ret + "</li>\n";
}
