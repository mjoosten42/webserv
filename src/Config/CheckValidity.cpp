#include "ConfigParser.hpp"

//  Parsing validity steps:
//  discard comments first (PARSING SIMPLIFICATION) //DONE
//  then check whether bracers are properly paired (INVALID_BRACES)
//  check whether no text between ';' and '}' or EOF (MISSING SEMICOLON)
//  finally check whether every simple directive has name and params (MISSING ARGS)

bool ConfigParser::check_validity(std::vector<std::string>& config_file) {
	discardComments(config_file);
	return (true);
}

void ConfigParser::discardComments(std::vector<std::string>& config) {
	std::vector<std::string>::iterator it;
	for (it = config.begin(); it != config.end(); ++it) {
		size_t pos = (*it).find_first_of(m_tokens[COMMENT]);
		if (pos != std::string::npos)
			(*it).resize(pos);
	}
}
