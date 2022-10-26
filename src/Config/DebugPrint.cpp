#include "ConfigParser.hpp"

std::string ConfigParser::getConfigAsString() {
	return getBlockAsString(m_main_context, "");
}

std::string ConfigParser::getSimpleAsString(t_simple_directive s, std::string tabs) {
	return tabs + s.name + "\t" + s.params + ";\n";
}

std::string ConfigParser::getBlockAsString(t_block_directive b, std::string tabs) {
	std::string block;

	block += tabs + b.name;
	if (!b.additional_params.empty())
		block += " " + b.additional_params;
	block += " {\n";

	std::vector<t_simple_directive>::iterator it_s;
	for (it_s = b.simple_directives.begin(); it_s != b.simple_directives.end(); ++it_s)
		block += getSimpleAsString(*it_s, tabs + "\t");

	std::vector<t_block_directive>::iterator it_b;
	for (it_b = b.block_directives.begin(); it_b != b.block_directives.end(); ++it_b)
		block += getBlockAsString(*it_b, tabs + "\t");
	block += tabs + "}\n";
	return block;
}
