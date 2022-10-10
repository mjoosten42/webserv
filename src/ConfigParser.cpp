#include "ConfigParser.hpp"

//  TOKENS: ' ' ';' '#' '{' '}'

//  Parsing validity steps:
//  discard comments first (PARSING SIMPLIFICATION)
//  then check whether bracers are properly paired (INVALID_BRACES)
//  check whether no text between ';' and '}' (MISSING SEMICOLON)
//  finally check whether every simple directive has name and params (MISSING ARGS)

ConfigParser::ConfigParser() {
	m_main_context.name = "main";
}

bool ConfigParser::parse_config(const char *path) {
	debug_print_config();
	return (true);
}

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
	std::cout << std::endl << "}";
}

void ConfigParser::debug_print_config() {
	debug_print_block(m_main_context);
}
