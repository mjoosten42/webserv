#include "ConfigParser.hpp"

std::string ConfigParser::getConfigAsString() {
	return getBlockAsString(m_main_context, "");
}

std::string ConfigParser::getSimpleAsString(const simple_directive &simple, const std::string &tabs) {
	return tabs + simple.name + "\t" + simple.params + ";\n";
}

std::string ConfigParser::getBlockAsString(const block_directive &block, const std::string &tabs) {
	std::string str = tabs + block.name;

	if (!block.params.empty())
		str += " " + block.params;
	str += " {\n";

	for (auto &simple : block.simple_directives)
		str += getSimpleAsString(simple, tabs + "\t");

	for (auto &sub_block : block.block_directives)
		str += getBlockAsString(sub_block, tabs + "\t");

	return str + tabs + "}\n";
}
