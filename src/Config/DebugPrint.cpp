#include "ConfigParser.hpp"

std::string ConfigParser::getConfigAsString() {
	return getBlockAsString(m_main_context, "");
}

std::string ConfigParser::getSimpleAsString(t_simple_directive simple, std::string tabs) {
	return tabs + simple.name + "\t" + simple.params + ";\n";
}

std::string ConfigParser::getBlockAsString(t_block_directive b, std::string tabs) {
	std::string str = tabs + b.name;

	if (!b.additional_params.empty())
		str += " " + b.additional_params;
	str += " {\n";

	for (auto &simple : b.simple_directives)
		str += getSimpleAsString(simple, tabs + "\t");

	for (auto &block : b.block_directives)
		str += getBlockAsString(block, tabs + "\t");

	return str + tabs + "}\n";
}
