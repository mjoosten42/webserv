#include "ConfigParser.hpp"

#include <iostream>

void ConfigParser::debug_print() {
	std::cout << "DEBUG PRINTING CONFIG FILE THAT HAS BEEN PARSED:" << std::endl;
	debug_print_block(m_main_context, "");
}

void ConfigParser::debug_print_simple(t_simple_directive s, std::string tabs) {
	std::cout << tabs << s.name << "\t" << s.params << ";" << std::endl;
}

void ConfigParser::debug_print_block(t_block_directive b, std::string tabs) {
	std::cout << tabs << b.name;
	if (!b.additional_params.empty())
		std::cout << " " << b.additional_params;
	std::cout << " {" << std::endl;

	std::vector<t_simple_directive>::iterator it_s;
	for (it_s = b.simple_directives.begin(); it_s != b.simple_directives.end(); ++it_s)
		debug_print_simple(*it_s, tabs + "\t");

	std::vector<t_block_directive>::iterator it_b;
	for (it_b = b.block_directives.begin(); it_b != b.block_directives.end(); ++it_b)
		debug_print_block(*it_b, tabs + "\t");
	std::cout << tabs << "}" << std::endl;
}
