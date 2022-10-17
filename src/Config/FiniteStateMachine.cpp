#include "ConfigParser.hpp"
#include "stringutils.hpp"

void ConfigParser::finite_state_machine(std::vector<std::string>& file) {
	t_block_directive				  *context = &m_main_context;
	std::vector<std::string>::iterator it;

	for (it = file.begin(); it != file.end(); ++it) {
		size_t pos = (*it).find_first_of(m_tokens, 0, SIZE);
		if (!(*it).empty()) {
			if (pos == std::string::npos)
				state_simpledirective(&context, it, file); //  Enables multi-line simple directives
			else {
				switch ((*it)[pos]) {
					case (';'):
						state_simpledirective(&context, it, file);
						break;
					case ('{'):
						state_openblock(&context, it);
						break;
					case ('}'):
						state_closeblock(&context);
						break;
					default:
						continue;
				}
			}
		}
	}
}

void ConfigParser::state_simpledirective(t_block_directive				   **context,
										 std::vector<std::string>::iterator& it,
										 std::vector<std::string>		   & file) {
	t_simple_directive	  tmp;
	std::string::iterator str_i = (*it).begin();
	std::string			 *field = &(tmp.name);
	while (*str_i != m_tokens[SEMICOLON] && str_i != (*it).end()) {
		if (std::isspace(*str_i) && field == &(tmp.name))
			field = &(tmp.params);
		*field = *field + *str_i;
		str_i++;
		if (str_i == (*it).end()) {
			++it;
			str_i = (*it).begin();
		}
	}
	tmp.params = trimLeadingWhiteSpace(tmp.params);
	if ((tmp.params).empty()) {
		std::string reason = "No parameters for directive \"" + tmp.name + "\"";
		throw_config_error(file, it, reason);
	}
	(*context)->simple_directives.push_back(tmp);
	return;
}

void ConfigParser::state_openblock(t_block_directive **context, std::vector<std::string>::iterator& it) {
	t_block_directive	  tmp;
	std::string::iterator str_i;
	std::string			 *field = &(tmp.name);
	tmp.parent_context			= *context;

	for (str_i = (*it).begin(); str_i != (*it).end() && *str_i != m_tokens[OPEN_BRACE]; ++str_i) {
		if (std::isspace(*str_i) && field == &(tmp.name))
			field = &(tmp.additional_params);
		*field = *field + *str_i;
	}
	tmp.additional_params = trimLeadingWhiteSpace(tmp.additional_params);
	tmp.additional_params = trimTrailingWhiteSpace(tmp.additional_params);
	(*context)->block_directives.push_back(tmp);
	(*context) = &((*context)->block_directives.back());
}

void ConfigParser::state_closeblock(t_block_directive **context) {
	if ((*context)->parent_context != NULL)
		(*context) = (*context)->parent_context;
}
