#include "AutoIndex.hpp"

// Returns the number of files and directories in the specified directory recursively.
// Stores the names of all files and directories in a string vector it is passed as a param.
// These stored names have leading tabs to indicate directory structure.
unsigned int recursiveFileCount(const std::string directory, std::vector<std::string>& file_structure, std::string tabulation) {
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
