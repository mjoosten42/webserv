#include "ConfigParser.hpp"
#include "defines.hpp" // IFS
#include "stringutils.hpp"
#include "utils.hpp"

#include <algorithm> //min
#include <stack>
#include <string>
#include <vector>

void ConfigParser::check_validity() {
	for (auto &line : config)
		trim(line, IFS);

	discard_comments();
	check_braces_error();
	check_semicolon_error();
}

void ConfigParser::check_braces_error() {
	std::stack<size_t> bracket_pairer;

	for (size_t i = 0; i != config.size(); i++) {
		for (auto c : config[i]) {
			if (c == '{')
				bracket_pairer.push(i);
			if (c == '}') {
				if (bracket_pairer.empty())
					throw_config_error(i, "No matching open brace for closing brace");
				bracket_pairer.pop();
			}
		}
	}
	if (!(bracket_pairer.empty()))
		throw_config_error(bracket_pairer.top(), "No matching close brace for opening brace");
	return;
}

void ConfigParser::check_semicolon_error() {
	std::stack<size_t> missing_semicolon;

	for (size_t i = 0; i != config.size(); i++) {
		std::string &line = config[i];
		size_t		 pos  = line.find_first_of(m_tokens, 0, SIZE);

		if (line.empty())
			continue;

		if (pos == std::string::npos)
			missing_semicolon.push(i);
		else {
			switch (line.at(pos)) {
				case (';'):
					while (!missing_semicolon.empty())
						missing_semicolon.pop();
					break;
				case ('{'):
				case ('}'):
					if (!missing_semicolon.empty())
						throw_config_error(missing_semicolon.top(), "No closing semicolon for statement");
					break;
				default:
					continue;
			}
		}
	}
}

void ConfigParser::discard_comments() {
	for (auto &line : config) {
		size_t pos = line.find(m_tokens[COMMENT]);
		if (pos != std::string::npos)
			line.erase(pos);
		trim(line, IFS);
	}
}

void ConfigParser::check_overflow_errors(size_t line, const simple_directive &to_check) {
	int val = 0;
	try {
		if (to_check.name == "listen")
			val = stringToIntegral<short>(to_check.params);
		if (to_check.name == "client_max_body_size")
			stringToIntegral<size_t>(to_check.params);
		if (to_check.name == "error_page") {
			std::vector<std::string> args = stringSplit(to_check.params);
			for (auto it = args.begin(); it < args.end(); it += 2)
				val = std::min((stringToIntegral<int>(*it)), val);
		}
	} catch (std::exception &e) {
		throw_config_error(line, "Value too large for \"" + to_check.name + "\" directive");
	}
	if (val < 0)
		throw_config_error(line, "Negative value for \"" + to_check.name + "\" directive");
	return;
}

void ConfigParser::throw_config_error(size_t line, const std::string &reason) {
	std::string error_line = toString(line + 1);
	std::string msg		   = "\nERROR: Invalid Config - " + reason;

	msg += " on line " + error_line;
	msg += "\nLINE " + error_line + ": " + config[line];
	throw(std::invalid_argument(msg));
}
