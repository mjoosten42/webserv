#include "AutoIndex.hpp"

#include "logger.hpp"
#include "stringutils.hpp"

// TODO: Still need to fix the behaviour when a directory is selected in the auto indexing.

// Returns the number of files and directories in the specified directory recursively.
// Stores the names of all files and directories in a string vector it is passed as a param.
// These stored names have leading tabs to indicate directory structure.
unsigned int
	recursiveFileCount(const std::string directory, std::vector<std::string>& file_structure, std::string tabulation) {
	DIR			*derp; // DirEctoRy Pointer
	unsigned int ret = 0;

	derp = opendir(directory.c_str());
	if (derp == NULL)
		return (ret);
	struct dirent *contents;
	while ((contents = readdir(derp)) != NULL) {
		std::string name = contents->d_name;
		if (name != "." && name != "..") {
			file_structure.push_back(tabulation + name);
			ret++;
			std::string next_dir = directory + "/" + name;
			ret					 = ret + recursiveFileCount(next_dir, file_structure, tabulation + "\t");
		}
	}
	closedir(derp);
	return (ret);
}

// Same as the above, except it saves a vector of paths relative to the site's root.
unsigned int
	recursivePathCount(const std::string directory, std::vector<std::string>& file_structure, std::string tabulation) {
	DIR			*derp; // DirEctoRy Pointer
	unsigned int ret = 0;

	derp = opendir(directory.c_str());
	if (derp == NULL)
		return (ret);
	struct dirent *contents;
	while ((contents = readdir(derp)) != NULL) {
		std::string name = contents->d_name;
		if (name != "." && name != "..") {
			file_structure.push_back(tabulation + name);
			ret++;
			std::string next_dir = directory + "/" + name;
			ret					 = ret + recursivePathCount(next_dir, file_structure, tabulation + name + "/");
		}
	}
	closedir(derp);
	return (ret);
}

//	Returns as a string the html body of a Response that indexes all files and sub directories of a given dir.
//	Params: the 'dir_path' passed must be a path from, and including, the server's root directory.
//	Params: the 'root' passed must be the root of the server that we are indexing relative to.
std::string autoIndexHtml(std::string dir_path, std::string root) {
	std::string relative_path = dir_path.substr(root.length());

	LOG("AutoIndexing...");
	std::vector<std::string> content_paths;
	recursivePathCount(dir_path, content_paths);
	std::vector<std::string>::iterator cp_it = content_paths.begin();

	std::vector<std::string> content_names;
	recursiveFileCount(dir_path, content_names);
	std::vector<std::string>::iterator cn_it = content_names.begin();

	std::string ret = "<h1> Index of directory: </h1>\n";
	ret += "<ul>";

	for (; cp_it != content_paths.end(); cp_it++) {
		ret += "<li><a href=\"" + relative_path + *cp_it + "\">";
		unsigned int tabs = countAndTrimLeadingWhiteSpace(*cn_it);
		for (unsigned int i = 0; i < tabs; i++)
			ret += "<ul>";
		ret = ret + *cn_it;
		for (unsigned int i = 0; i < tabs; i++)
			ret = ret + "</ul>";
		ret += "</a></li>\n";
		cn_it++;
	}
	ret += "</ul>\n";
	return (ret);
}
