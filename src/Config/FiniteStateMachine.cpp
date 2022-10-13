#include "ConfigParser.hpp"
#include "stringutils.hpp"

void ConfigParser::finite_state_machine(std::vector<std::string>& file) {
	t_block_directive				  *context = &m_main_context;
	std::vector<std::string>::iterator it;

	for (it = file.begin(); it != file.end(); ++it) {
		size_t pos = (*it).find_first_of(m_tokens, 0, SIZE);
		if (!(*it).empty()) {
			switch ((*it)[pos]) {
				case (';'):
					state_simpledirective(&context, it);
					break;
				case ('{'):
					state_openblock(&context, it);
					break;
				case ('}'):
					state_closeblock(&context, it);
					break;
				default:
					state_simpledirective(&context, it); //  Enables multi-line simple directives
					continue;
			}
		}
	}
}

void ConfigParser::state_simpledirective(t_block_directive **context, std::vector<std::string>::iterator& it) {
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
	(*context)->simple_directives.push_back(tmp);
	return;
}

void ConfigParser::state_openblock(t_block_directive **context, std::vector<std::string>::iterator& it) {
	t_block_directive tmp;
	tmp.parent_context = *context;
	std::string::iterator str_i;
	for (str_i = (*it).begin(); str_i != (*it).end() && !std::isspace(*str_i); ++str_i)
		tmp.name = tmp.name + *str_i;
	(*context)->block_directives.push_back(tmp);
	(*context) = &((*context)->block_directives.back());
}

void ConfigParser::state_closeblock(t_block_directive **context, std::vector<std::string>::iterator& it) {
	if ((*context)->parent_context != NULL)
		(*context) = (*context)->parent_context;
	(void)it;
}
