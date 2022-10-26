#include <vector>
#include <string>
#include <dirent.h>

unsigned int recursiveFileCount(const std::string		  directory,
								std::vector<std::string>& file_structure,
								std::string				  tabulation = "");

unsigned int recursivePathCount(const std::string		  directory,
								std::vector<std::string>& file_structure,
								std::string				  tabulation = "");

