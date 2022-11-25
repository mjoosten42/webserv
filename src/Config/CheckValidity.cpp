#include "ConfigParser.hpp"
#include "defines.hpp" // IFS
#include "stringutils.hpp"
#include "utils.hpp"

#include <stack>
#include <string>
#include <vector>

void ConfigParser::check_validity(std::vector<std::string> &config) {
	for (auto &line : config)
		trim(line, IFS);

	discard_comments(config);
	discard_empty(config);
	check_braces_error(config);
	check_semicolon_error(config);
}

void ConfigParser::check_braces_error(std::vector<std::string> &config) {
	std::stack<std::vector<std::string>::iterator> bracket_pairer;

	for (auto file_it = config.begin(); file_it != config.end(); ++file_it) {
		for (auto str_it = file_it->begin(); str_it != file_it->end(); ++str_it) {
			if (*str_it == '{')
				bracket_pairer.push(file_it);
			if (*str_it == '}') {
				if (bracket_pairer.empty())
					throw_config_error(config, file_it, "No matching open brace for closing brace");
				bracket_pairer.pop();
			}
		}
	}
	if (!(bracket_pairer.empty()))
		throw_config_error(config, bracket_pairer.top(), "No matching close brace for opening brace");
	return;
}

void ConfigParser::check_semicolon_error(std::vector<std::string> &config) {
	std::stack<std::vector<std::string>::iterator> missing_semicolon;

	for (auto file_it = config.begin(); file_it != config.end(); ++file_it) {
		size_t pos = file_it->find_first_of(m_tokens, 0, SIZE);
		if (file_it->empty())
			continue;
		if (pos == std::string::npos)
			missing_semicolon.push(file_it);
		else {
			switch (file_it->at(pos)) {
				case (';'):
					while (!missing_semicolon.empty())
						missing_semicolon.pop();
					break;
				case ('{'):
				case ('}'):
					if (!missing_semicolon.empty()) {
						file_it = missing_semicolon.top();
						throw_config_error(config, file_it, "No closing semicolon for statement");
					}
					break;
				default:
					continue;
			}
		}
	}
}

void ConfigParser::discard_comments(std::vector<std::string> &config) {
	for (auto &line : config) {
		size_t pos = line.find_first_of(m_tokens[COMMENT]);
		if (pos != std::string::npos)
			line.erase(pos);
	}
}

void ConfigParser::discard_empty(std::vector<std::string> &config) {
	for (auto it = config.begin(); it != config.end(); it++) {
		size_t pos = it->find_first_not_of(IFS);
		if (pos == std::string::npos)
			config.erase(it--);
	}
}

void ConfigParser::check_overflow_errors(std::vector<std::string>			&config,
										 std::vector<std::string>::iterator &file_it,
										 const t_simple_directive			&to_check) {
	try {
		if (to_check.name == "listen")
			stringToIntegral<short>(to_check.params);
		if (to_check.name == "client_max_body_size")
			stringToIntegral<size_t>(to_check.params);
		if (to_check.name == "error_page") {
			std::vector<std::string> args = stringSplit(to_check.params);
			for (auto it = args.begin(); it < args.end(); it += 2)
				stringToIntegral<int>(*it);
		}
	} catch (std::exception &e) {
		throw_config_error(config, file_it, "Value too large (or negative) for \"" + to_check.name + "\" directive");
	}
	return;
}

void ConfigParser::throw_config_error(std::vector<std::string>			 &config,
									  std::vector<std::string>::iterator &file_it,
									  const std::string					 &reason) {
	std::string error_line = toString(file_it - config.begin() + 1);
	std::string msg		   = "\nERROR: Invalid Config - " + reason;
	msg += " on line " + error_line;
	msg += "\nLINE " + error_line + ": " + *file_it;
	throw(std::invalid_argument(msg));
}
