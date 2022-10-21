#include "ConfigParser.hpp"
#include "utils.hpp"

#include <fstream>

ConfigParser::ConfigParser() {
	m_main_context.name			  = "main";
	m_main_context.parent_context = NULL;
	m_tokens[SEMICOLON]			  = ';';
	m_tokens[COMMENT]			  = '#';
	m_tokens[OPEN_BRACE]		  = '{';
	m_tokens[CLOSE_BRACE]		  = '}';
}

bool ConfigParser::parse_config(const char *path) {
	std::vector<std::string> config_file = readFile(path);
	check_validity(config_file); //  Will throw exception incase of invalid config.
	finite_state_machine(config_file);
	//  debug_print(); //  If you want to see the parsed contents.
	return (true);
}

std::vector<std::string> ConfigParser::readFile(const char *path) {
	std::ifstream			 conf_stream(path);
	std::vector<std::string> ret;
	if (conf_stream.is_open()) {
		while (conf_stream.good()) {
			std::string tmp;
			std::getline(conf_stream, tmp);
			ret.push_back(tmp);
		}
	}
	return (ret);
}

//  Returns a vector of pointers to all t_block_directives who's name matches 'blocks_to_fetch'.
//  Only searches the context of the t_block_directive object this function was called on and its sub directives.
std::vector<t_block_directive *> s_block_directive::fetch_matching_blocks(const std::string& blocks_to_fetch) {
	std::vector<t_block_directive *> ret;
	recurse_blocks(ret, blocks_to_fetch);
	return (ret);
}

void s_block_directive::recurse_blocks(std::vector<t_block_directive *>& ret, const std::string& blocks_to_fetch) {
	std::vector<t_block_directive>::iterator it_b;
	for (it_b = this->block_directives.begin(); it_b != this->block_directives.end(); ++it_b) {
		if ((*it_b).name.compare(blocks_to_fetch) == 0)
			ret.push_back(&(*it_b));
		(*it_b).recurse_blocks(ret, blocks_to_fetch);
	}
	return;
}

//  Returns first params associated with the requested simple_directive, if it is present in the
//  block_directive this method was called on.
//	Otherwise returns empty string.
std::string s_block_directive::fetch_simple(const std::string& key) {
	std::vector<t_simple_directive>::iterator it_s;
	for (it_s = this->simple_directives.begin(); it_s != this->simple_directives.end(); ++it_s)
		if ((*it_s).name.compare(key) == 0)
			return ((*it_s).params);
	return ("");
}

//  Returns a vector of params of all the instances of simple_directive with the given key.
//  When nothing found, returns an empty vector.
std::vector<std::string> s_block_directive::fetch_simple_all(const std::string& key) {
	std::vector<std::string>				  ret;
	std::vector<t_simple_directive>::iterator it_s;
	for (it_s = this->simple_directives.begin(); it_s != this->simple_directives.end(); ++it_s)
		if ((*it_s).name.compare(key) == 0)
			ret.push_back((*it_s).params);
	return (ret);
}

//  Returns first params associated with the requested simple_directive, if it is present in the
//  block_directive this method was called on OR ANY OF THE BLOCK DIRECTIVES IT CONTAINS.
//	Otherwise returns empty string.
std::string s_block_directive::fetch_simple_recursive(const std::string& key) {
	std::string								  ret = "";
	std::vector<t_simple_directive>::iterator it_s;
	for (it_s = this->simple_directives.begin(); it_s != this->simple_directives.end(); ++it_s)
		if ((*it_s).name.compare(key) == 0)
			return ((*it_s).params);

	std::vector<t_block_directive>::iterator it_b;
	for (it_b = this->block_directives.begin(); it_b != this->block_directives.end(); ++it_b) {
		ret = it_b->fetch_simple_recursive(key);
		if (!ret.empty())
			return (ret);
	}
	return (ret);
}
