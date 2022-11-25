#include "ConfigParser.hpp"
#include "utils.hpp"

#include <fstream>

char ConfigParser::m_tokens[SIZE] = { ';', '#', '{', '}' };

ConfigParser::ConfigParser(const char *path) {
	m_main_context.name = "main";
	m_main_context.parent_context = NULL;
	
	readFile(path);
	check_validity(); // Will throw exception incase of invalid config.
	finite_state_machine();
}

ConfigParser::ConfigParser(const std::string& data) {
	std::stringstream stream(data);
	std::string line;
	while (std::getline(stream, line))
		config.push_back(line);

	check_validity(); // Will throw exception incase of invalid config.
	finite_state_machine();
}

void ConfigParser::readFile(const char *path) {
	std::ifstream conf_stream(path);

	if (!conf_stream.is_open()) {
		std::string error = "open: ";
		error += strerror(errno);
		throw std::invalid_argument(error);
	}

	while (conf_stream.good()) {
		config.push_back("");
		std::getline(conf_stream, config.back());
	}
}

// Returns a vector of pointers to all block_directives who's name matches 'blocks_to_fetch'.
// Only searches the context of the block_directive object this function was called on and its sub directives.
std::vector<block_directive *> block_directive::fetch_matching_blocks(const std::string &blocks_to_fetch) {
	std::vector<block_directive *> ret;
	recurse_blocks(ret, blocks_to_fetch);
	return (ret);
}

void block_directive::recurse_blocks(std::vector<block_directive *> &ret, const std::string &blocks_to_fetch) {
	for (auto &block : block_directives) {
		if (!block.name.compare(blocks_to_fetch))
			ret.push_back(&block);
		block.recurse_blocks(ret, blocks_to_fetch);
	}
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
