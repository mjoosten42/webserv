#include "AutoIndex.hpp"

#include "logger.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

std::vector<Entry> recursivePathCount(const std::string directory) {
	std::vector<Entry> paths;
	Entry			   entry;
	std::string		   name;
	DIR				  *derp; // DirEctoRy Pointer
	struct dirent	  *contents;

	derp = opendir(directory.c_str());
	if (derp == NULL)
		return (paths);
	while ((contents = readdir(derp)) != NULL) {
		name = contents->d_name;
		if (name.back() == '.') // Skip current, up and hidden
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

//	Returns as a string the html body of a Response that indexes all files and sub directories of a given dir.
//	Params: the 'dir_path' passed must be a path from, and including, the server's root directory.
std::string autoIndexHtml(std::string dir_path) {
	LOG("AutoIndexing...");

	std::vector<Entry> content_paths = recursivePathCount(dir_path);
	std::string		   ret			 = "<h1> Index of directory: </h1>\n<ul>";

	for (size_t i = 0; i < content_paths.size(); i++)
		ret += content_paths[i].toString();

	return ret;
}

std::string Entry::toString() const {
	std::string ret = "<li>";

	ret += "<a href=\"" + name + "\">" + name + "</a>\n";

	if (!subdir.empty()) {
		ret += "<ul>\n";
		for (size_t i = 0; i < subdir.size(); i++)
			ret += subdir[i].toString();
		ret += "</ul>\n";
	}

	return ret + "</li>";
}

std::string basename(const std::string& path) {
	size_t pos = path.find_last_of("/");

	if (pos != std::string::npos)
		return path.substr(pos);
	return path;
}
