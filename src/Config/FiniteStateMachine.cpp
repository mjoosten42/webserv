#include "ConfigParser.hpp"
#include "defines.hpp"	   // IFS
#include "logger.hpp"	   // TODO
#include "stringutils.hpp" // trim

void ConfigParser::finite_state_machine() {
	block_directive *context = &m_main_context;

	for (size_t i = 0; i != config.size(); i++) {
		std::string &line = config[i];
		size_t		 pos  = line.find_first_of(m_tokens, 0, SIZE);

		if (line.empty())
			continue;

		if (pos == std::string::npos)
			throw_config_error(i, "Missing semicolon");

		switch (line[pos]) {
			case (';'):
				line.erase(pos);
				state_simpledirective(&context, line, i);
				break;
			case ('{'):
				state_openblock(&context, line, i);
				break;
			case ('}'):
				state_closeblock(&context);
				break;
			default:
				continue;
		}
	}
}

void ConfigParser::state_simpledirective(block_directive **context, std::string &line, size_t i) {
	size_t			 pos = line.find_first_of(IFS);
	simple_directive tmp;

	tmp.name = line.substr(0, pos);
	strToLower(tmp.name);

	if (pos != std::string::npos)
		tmp.params = line.substr(pos);
	trim(tmp.params, IFS);

	if (tmp.params.empty())
		throw_config_error(i, "Missing directive parameters");
	check_overflow_errors(i, tmp);
	(*context)->simple_directives.push_back(tmp);
}

void ConfigParser::state_openblock(block_directive **context, std::string &line, size_t i) {
	size_t			pos = line.find_first_of(IFS);
	block_directive tmp;

	if (line.back() != '{')
		throw_config_error(i, "Info after brace");
	line.pop_back(); // remove brace

	tmp.name = line.substr(0, pos);
	strToLower(tmp.name);

	tmp.parent_context = *context;

	if (pos != std::string::npos)
		tmp.params = line.substr(pos);
	trim(tmp.params, IFS);

	(*context)->block_directives.push_back(tmp);
	(*context) = &((*context)->block_directives.back());
}

void ConfigParser::state_closeblock(block_directive **context) {
	if ((*context)->parent_context != NULL)
		(*context) = (*context)->parent_context;
}
