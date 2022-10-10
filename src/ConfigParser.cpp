#include "ConfigParser.hpp"

#include "utils.hpp"

//  GENERAL ORGANISATION:

ConfigParser::ConfigParser() {
	m_main_context.name = "main";
}

bool ConfigParser::parse_config(const char *path) {
	std::vector<std::string> config_file = loadConfigToStrVector(path);
	discardComments(config_file);
	//  std::vector<std::string>::iterator it;
	//  for (it = conf_vector.begin(); it != conf_vector.end(); ++it) {
	//          print(*it);
	//  }

	debug_print_config();
	return (true);
}

//  READING FILE AND CONFIRMING VALIDITY:

//  Parsing validity steps:
//  discard comments first (PARSING SIMPLIFICATION) //DONE
//  then check whether bracers are properly paired (INVALID_BRACES)
//  check whether no text between ';' and '}' or EOF (MISSING SEMICOLON)
//  finally check whether every simple directive has name and params (MISSING ARGS)

std::vector<std::string> ConfigParser::loadConfigToStrVector(const char *path) {
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

void ConfigParser::discardComments(std::vector<std::string>& config) {
	std::vector<std::string>::iterator it;
	for (it = config.begin(); it != config.end(); ++it) {
		size_t pos = (*it).find_first_of('#');
		if (pos != std::string::npos)
			(*it).resize(pos);
	}
}

//  STORING TO PARSING STRUCT

//  Tokens to consider: ' ' ';' '#' '{' '}'

//  DEBUG PRINTING:

void ConfigParser::debug_print_simple(t_simple_directive s) {
	std::cout << s.name << "   " << s.params << ";" << std::endl;
}

void ConfigParser::debug_print_block(t_block_directive b) {
	std::cout << b.name << " {" << std::endl;

	std::vector<t_simple_directive>::iterator it_s;
	for (it_s = b.simple_directives.begin(); it_s != b.simple_directives.end(); ++it_s)
		debug_print_simple(*it_s);
	std::vector<t_block_directive>::iterator it_b;
	for (it_b = b.block_directives.begin(); it_b != b.block_directives.end(); ++it_b)
		debug_print_block(*it_b);
	std::cout << std::endl << "}" << std::endl;
}

void ConfigParser::debug_print_config() {
	std::cout << "DEBUG PRINTING CONFIG FILE THAT HAS BEEN PARSED:" << std::endl;
	debug_print_block(m_main_context);
}
