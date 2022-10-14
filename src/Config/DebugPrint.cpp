#include "ConfigParser.hpp"

void ConfigParser::debug_print() {
	std::cout << "DEBUG PRINTING CONFIG FILE THAT HAS BEEN PARSED:" << std::endl;
	debug_print_block(m_main_context, "");
}

void ConfigParser::debug_print_simple(t_simple_directive s, std::string tabs) {
	std::cout << tabs << s.name << "	" << s.params << ";" << std::endl;
}

void ConfigParser::debug_print_block(t_block_directive b, std::string tabs) {
	if (b.additional_params.empty())
		std::cout << tabs << b.name << "{" << std::endl << tabs;
	else
		std::cout << tabs << b.name << " " << b.additional_params << "{" << std::endl
				  << tabs;
	tabs = tabs + "   ";

	std::vector<t_simple_directive>::iterator it_s;
	for (it_s = b.simple_directives.begin(); it_s != b.simple_directives.end(); ++it_s)
		debug_print_simple(*it_s, tabs);

	std::vector<t_block_directive>::iterator it_b;
	for (it_b = b.block_directives.begin(); it_b != b.block_directives.end(); ++it_b)
		debug_print_block(*it_b, tabs);
	std::cout << tabs.substr(3) << "}" << std::endl;
}
