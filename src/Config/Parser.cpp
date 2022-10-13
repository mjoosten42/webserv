#include "ConfigParser.hpp"
#include "utils.hpp"

//  TODO: Make handler functions that return specific parts of the config e.g. all the servers.

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
	debug_print(); //  If you want to see the parsed contents.
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
