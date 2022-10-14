#include "ConfigParser.hpp"
#include "stringutils.hpp"

#include <stack>
#include <string>
#include <vector>

void ConfigParser::check_validity(std::vector<std::string>& config) {
	discard_comments(config);
	check_braces_error(config);
	check_semicolon_error(config);
	return;
}

void ConfigParser::check_braces_error(std::vector<std::string>& config) {
	std::string::iterator						   str_it;
	std::vector<std::string>::iterator			   file_it;
	std::stack<std::vector<std::string>::iterator> bracket_pairer;
	for (file_it = config.begin(); file_it != config.end(); ++file_it) {
		for (str_it = (*file_it).begin(); str_it != (*file_it).end(); ++str_it) {
			if (*str_it == '{')
				bracket_pairer.push(file_it);
			if (*str_it == '}' && bracket_pairer.empty())
				throw_config_error(config, file_it, "No matching open brace for closing brace");
			if (*str_it == '}')
				bracket_pairer.pop();
		}
	}
	if (!(bracket_pairer.empty())) {
		file_it = bracket_pairer.top();
		throw_config_error(config, file_it, "No matching close brace for opening brace");
	}
	return;
}

void ConfigParser::check_semicolon_error(std::vector<std::string>& config) {
	std::vector<std::string>::iterator			   file_it;
	std::stack<std::vector<std::string>::iterator> missing_semicolon;

	for (file_it = config.begin(); file_it != config.end(); ++file_it) {
		*file_it   = trimLeadingWhiteSpace(*file_it);
		size_t pos = (*file_it).find_first_of(m_tokens, 0, SIZE);
		switch ((*file_it)[pos]) {
			case (';'):
				while (!missing_semicolon.empty())
					missing_semicolon.pop();
				break;
			case ('{'):
				if (!missing_semicolon.empty()) {
					file_it = missing_semicolon.top();
					throw_config_error(config, file_it, "No closing semicolon for statement");
				}
				break;
			case ('}'):
				if (!missing_semicolon.empty()) {
					file_it = missing_semicolon.top();
					throw_config_error(config, file_it, "No closing semicolon for statement");
				}
				break;
			default:
				if (!(*file_it).empty())
					missing_semicolon.push(file_it);
				continue;
		}
	}
	return;
}

void ConfigParser::discard_comments(std::vector<std::string>& config) {
	std::vector<std::string>::iterator it;
	for (it = config.begin(); it != config.end(); ++it) {
		size_t pos = (*it).find_first_of(m_tokens[COMMENT]);
		if (pos != std::string::npos)
			(*it).resize(pos);
	}
}

void ConfigParser::throw_config_error(std::vector<std::string>			& config,
									  std::vector<std::string>::iterator& file_it,
									  std::string						  reason) {
	std::string error_line = toString(static_cast<int>(file_it - config.begin() + 1));
	std::string msg		   = "\nERROR: Invalid Config - " + reason;
	msg					   = msg + " on line " + error_line;
	msg					   = msg + "\nLINE " + error_line + ": " + *file_it;
	throw(std::invalid_argument(msg));
	return;
}