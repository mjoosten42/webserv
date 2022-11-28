#include "ConfigParser.hpp"
#include "defines.hpp"	   // IFS
#include "logger.hpp"	   // TODO
#include "stringutils.hpp" // trim

const static char *directives[] = {
	"listen",		"server_name",		   "cgi", "root", "index", "upload", "redirect", "autoindex", "error_page",
	"limit_except", "client_max_body_size"
};

void ConfigParser::finite_state_machine() {
	block_directive *context = &m_main_context;

	for (size_t i = 0; i != config.size(); i++) {
		std::string &line = config[i];
		size_t		 pos  = line.find_first_of(m_tokens, 0, SIZE);

		if (pos == std::string::npos)
			throw_config_error(i, "Missing semicolon");

		char c = line[pos];
		line.erase(pos);
		switch (c) {
			case (';'):
				state_simpledirective(&context, line, i);
				break;
			case ('{'):
				state_openblock(&context, line);
				break;
			case ('}'):
				state_closeblock(&context);
				break;
			default:
				continue;
		}
	}
}

void ConfigParser::state_simpledirective(block_directive **context, const std::string &line, size_t i) {
	simple_directive tmp;
	size_t			 pos = line.find_first_of(IFS);

	tmp.name = line.substr(0, pos);

	if (!isAllowedDirective(tmp.name))
		throw_config_error(i, "Unknown directive");

	if (pos != std::string::npos) {
		pos = line.find_first_not_of(IFS, pos);
		if (pos != std::string::npos) {
			tmp.params = line.substr(pos);
			trim(tmp.params, IFS);
		}
	}
	if (tmp.params.empty())
		throw_config_error(i, "Missing directive parameters");
	check_overflow_errors(i, tmp);
	(*context)->simple_directives.push_back(tmp);
}

void ConfigParser::state_openblock(block_directive **context, const std::string &line) {
	block_directive				tmp;
	std::string::const_iterator str_i = line.begin();
	std::string				   *field = &(tmp.name);
	tmp.parent_context				  = *context;

	for (; str_i != line.end() && *str_i != m_tokens[OPEN_BRACE]; ++str_i) {
		if (std::isspace(*str_i) && field == &(tmp.name))
			field = &(tmp.additional_params);
		*field += *str_i;
	}
	trim(tmp.additional_params, IFS);
	(*context)->block_directives.push_back(tmp);
	(*context) = &((*context)->block_directives.back());
}

void ConfigParser::state_closeblock(block_directive **context) {
	if ((*context)->parent_context != NULL)
		(*context) = (*context)->parent_context;
}

bool ConfigParser::isAllowedDirective(const std::string &str) const {
	for (size_t i = 0; i < SIZEOF_ARRAY(directives); i++)
		if (str == directives[i])
			return true;
	return false;
}
