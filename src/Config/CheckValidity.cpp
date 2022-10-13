#include "ConfigParser.hpp"
#include "stringutils.hpp"

#include <stack>
#include <string>
#include <vector>

//  Parsing validity steps:
//  discard comments first (PARSING SIMPLIFICATION) //DONE
//  then check whether bracers are properly paired (INVALID_BRACES) //DONE
//  check whether no text between ';' and '}' or EOF (MISSING SEMICOLON)
//  finally check whether every simple directive has name and params (MISSING ARGS)

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
			if (*str_it == '}' && bracket_pairer.empty()) {
				std::string error_line = toString(static_cast<int>(file_it - config.begin() + 1));
				std::string msg		   = "\nERROR: Invalid Config - No matching open brace";
				msg					   = msg + " for closing brace on line " + error_line;
				msg					   = msg + "\nLINE " + error_line + ": " + *file_it;
				throw(std::invalid_argument(msg));
			}
			if (*str_it == '}')
				bracket_pairer.pop();
		}
	}
	if (!(bracket_pairer.empty())) {
		file_it				   = bracket_pairer.top();
		std::string error_line = toString(static_cast<int>(file_it - config.begin() + 1));
		std::string msg		   = "\nERROR: Invalid Config - No matching close brace";
		msg					   = msg + " for opening brace on line " + error_line;
		msg					   = msg + "\nLINE " + error_line + ": " + *file_it;
		throw(std::invalid_argument(msg));
	}
	return;
}

void ConfigParser::check_semicolon_error(std::vector<std::string>& config) {
	std::vector<std::string>::iterator			   file_it;
	std::stack<std::vector<std::string>::iterator> missing_semicolon;

	for (file_it = config.begin(); file_it != config.end(); ++file_it) {
		*file_it = trimLeadingWhiteSpace(*file_it);
		if (!(*file_it).empty()) { //  Might be superfluous
			size_t pos = (*file_it).find_first_of(m_tokens, 0, SIZE);
			switch ((*file_it)[pos]) {
				case (';'):
					while (!missing_semicolon.empty())
						missing_semicolon.pop();
					break;
				case ('{'):
					if (!missing_semicolon.empty()) {
						file_it				   = missing_semicolon.top();
						std::string error_line = toString(static_cast<int>(file_it - config.begin() + 1));
						std::string msg		   = "\nERROR: Invalid Config - No closing semicolon";
						msg					   = msg + " for statement on line " + error_line;
						msg					   = msg + "\nLINE " + error_line + ": " + *file_it;
						throw(std::invalid_argument(msg));
					}
					break;
				case ('}'):
					if (!missing_semicolon.empty()) {
						file_it				   = missing_semicolon.top();
						std::string error_line = toString(static_cast<int>(file_it - config.begin() + 1));
						std::string msg		   = "\nERROR: Invalid Config - No closing semicolon";
						msg					   = msg + " for statement on line " + error_line;
						msg					   = msg + "\nLINE " + error_line + ": " + *file_it;
						throw(std::invalid_argument(msg));
					}
					break;
				default:
					missing_semicolon.push(file_it);
					continue;
			}
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
