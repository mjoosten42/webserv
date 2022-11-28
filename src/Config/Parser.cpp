#include "ConfigParser.hpp"
#include "utils.hpp"

#include <fstream>
#include <istream>
#include <string>
#include <vector>

char ConfigParser::m_tokens[SIZE] = { ';', '#', '{', '}' };

ConfigParser::ConfigParser(const char *path) {
	config = readFile(path);
	init();
}

ConfigParser::ConfigParser(const std::string &data) {
	config = readData(data);
	init();
}

void ConfigParser::init() {
	m_main_context.name			  = "main";
	m_main_context.parent_context = NULL;

	check_validity(); // Will throw exception incase of invalid config.
	finite_state_machine();
}

#include "logger.hpp"

std::vector<std::string> ConfigParser::readData(const std::string &data) {
	std::vector<std::string> ret;
	std::istringstream		 stream(data);
	std::string				 line;

	while (std::getline(stream, line, '\n'))
		ret.push_back(line);
	return ret;
}

std::vector<std::string> ConfigParser::readFile(const char *path) {
	std::vector<std::string> ret;
	std::ifstream			 conf_stream(path);
	std::string				 line;

	if (!conf_stream.is_open())
		throw std::invalid_argument("File could not be opened");

	while (conf_stream.good()) {
		std::getline(conf_stream, line);
		ret.push_back(line);
	}
	return ret;
}

// Returns a vector of pointers to all block_directives who's name matches 'blocks_to_fetch'.
// Only searches the context of the block_directive object this function was called on and its sub directives.
std::vector<block_directive *> block_directive::fetch_matching_blocks(const std::string &blocks_to_fetch) {
	std::vector<block_directive *> ret;
	std::vector<block_directive *> tmp;

	for (auto &block : block_directives) {
		if (block.name == blocks_to_fetch)
			ret.push_back(&block);
		tmp = block.fetch_matching_blocks(blocks_to_fetch);
		ret.insert(ret.end(), tmp.begin(), tmp.end()); // Append tmp to ret
	}

	return (ret);
}

// Returns first params associated with the requested simple_directive, if it is present in the
// block_directive this method was called on.
// Otherwise returns empty string.
std::string block_directive::fetch_simple(const std::string &key) {
	for (auto &simple : simple_directives)
		if (simple.name == key)
			return (simple.params);
	return ("");
}
