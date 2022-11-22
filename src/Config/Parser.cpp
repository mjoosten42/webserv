#include "ConfigParser.hpp"
#include "utils.hpp"

#include <fstream>

ConfigParser::ConfigParser(const char *path) {
	m_main_context.name			  = "main";
	m_main_context.parent_context = NULL;
	m_tokens[SEMICOLON]			  = ';';
	m_tokens[COMMENT]			  = '#';
	m_tokens[OPEN_BRACE]		  = '{';
	m_tokens[CLOSE_BRACE]		  = '}';

	std::vector<std::string> config_file = readFile(path);
	check_validity(config_file); // Will throw exception incase of invalid config.
	finite_state_machine(config_file);
}

std::vector<std::string> ConfigParser::readFile(const char *path) {
	std::ifstream			 conf_stream(path);
	std::vector<std::string> ret;
	if (conf_stream.is_open()) {
		while (conf_stream.good()) {
			ret.push_back("");
			std::getline(conf_stream, ret.back());
		}
	}
	return (ret);
}

// Returns a vector of pointers to all t_block_directives who's name matches 'blocks_to_fetch'.
// Only searches the context of the t_block_directive object this function was called on and its sub directives.
std::vector<t_block_directive *> s_block_directive::fetch_matching_blocks(const std::string &blocks_to_fetch) {
	std::vector<t_block_directive *> ret;
	recurse_blocks(ret, blocks_to_fetch);
	return (ret);
}

void s_block_directive::recurse_blocks(std::vector<t_block_directive *> &ret, const std::string &blocks_to_fetch) {
	for (auto &block : block_directives) {
		if (!block.name.compare(blocks_to_fetch))
			ret.push_back(&block);
		block.recurse_blocks(ret, blocks_to_fetch);
	}
}

// Returns first params associated with the requested simple_directive, if it is present in the
// block_directive this method was called on.
// Otherwise returns empty string.
std::string s_block_directive::fetch_simple(const std::string &key) {
	for (auto &simple : simple_directives)
		if (simple.name == key)
			return (simple.params);
	return ("");
}
