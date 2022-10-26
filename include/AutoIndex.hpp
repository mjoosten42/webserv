#pragma once

#include <vector>
#include <string>

unsigned int recursiveFileCount(const std::string		  directory,
								std::vector<std::string>& file_structure,
								std::string				  tabulation = "");

unsigned int recursivePathCount(const std::string		  directory,
								std::vector<std::string>& file_structure,
								std::string				  tabulation = "");

std::string	autoIndexHtml(std::string dir_path);
