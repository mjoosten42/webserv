#include "ConfigParser.hpp"

#include "stringutils.hpp"
#include "utils.hpp"

//  GENERAL ORGANISATION:

ConfigParser::ConfigParser() {
	m_main_context.name			  = "main";
	m_main_context.parent_context = NULL;
	m_tokens[SPACE]				  = ' ';
	m_tokens[SEMICOLON]			  = ';';
	m_tokens[COMMENT]			  = '#';
	m_tokens[OPEN_BRACE]		  = '{';
	m_tokens[CLOSE_BRACE]		  = '}';
}

bool ConfigParser::parse_config(const char *path) {
	std::vector<std::string> config_file = loadConfigToStrVector(path);
	discardComments(config_file);
	finite_state_machine(config_file);
	//  std::vector<std::string>::iterator it;
	//  for (it = config_file.begin(); it != config_file.end(); ++it) {
	//          print(*it);
	//  }

	debug_print_config();
	return (true);
}

//  READING FILE AND CONFIRMING VALIDITY:

//  Parsing validity steps:
//  discard comments first (PARSING SIMPLIFICATION) //DONE
//  then check whether bracers are properly paired (INVALID_BRACES)
//  check whether no text between ';' and '}' or EOF (MISSING SEMICOLON)
//  finally check whether every simple directive has name and params (MISSING ARGS)

std::vector<std::string> ConfigParser::loadConfigToStrVector(const char *path) {
	std::ifstream			 conf_stream(path);
	std::vector<std::string> ret;
	if (conf_stream.is_open()) {
		while (conf_stream.good()) {
			std::string tmp;
			std::getline(conf_stream, tmp);
			ret.push_back(tmp);
		}
	}
	return (ret);
}

void ConfigParser::discardComments(std::vector<std::string>& config) {
	std::vector<std::string>::iterator it;
	for (it = config.begin(); it != config.end(); ++it) {
		size_t pos = (*it).find_first_of(m_tokens[COMMENT]);
		if (pos != std::string::npos)
			(*it).resize(pos);
	}
}

//  FINITE STATE MACHINE:

void ConfigParser::finite_state_machine(std::vector<std::string>& file) {
	t_block_directive				  *context = &m_main_context;
	std::vector<std::string>::iterator it;

	for (it = file.begin(); it != file.end(); ++it) {
		*it		   = trimLeadingWhiteSpace(*it);
		size_t pos = (*it).find_first_of(m_tokens + SEMICOLON, 0, SIZE); //  Find first token in line that isn't a space
		if (pos != std::string::npos) {
			//  print((*it)[pos]);
			//  print(context->name);
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
					continue; //  Pointer to current state and run that function?
			}
		}
	}
}

void ConfigParser::state_simpledirective(t_block_directive **context, std::vector<std::string>::iterator it) {
	t_simple_directive	  tmp;
	std::string::iterator str_i = (*it).begin();
	std::string			 *field = &(tmp.name);
	while (*str_i != m_tokens[SEMICOLON]) {
		if (*str_i == m_tokens[SPACE] && field == &(tmp.name))
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

void ConfigParser::state_openblock(t_block_directive **context, std::vector<std::string>::iterator it) {
	t_block_directive tmp;
	tmp.parent_context = *context;
	std::string::iterator str_i;
	for (str_i = (*it).begin(); str_i != (*it).end() && *str_i != m_tokens[SPACE]; ++str_i)
		tmp.name = tmp.name + *str_i;
	(*context)->block_directives.push_back(tmp);
	(*context) = &((*context)->block_directives.back());
	++it;
}

void ConfigParser::state_closeblock(t_block_directive **context, std::vector<std::string>::iterator it) {
	(*context) = (*context)->parent_context;
	++it;
}

//  DEBUG PRINTING:

void ConfigParser::debug_print_simple(t_simple_directive s, std::string tabs) {
	std::cout << tabs << "Name: |" << s.name << "| Params: |" << s.params << "|;" << std::endl;
}

void ConfigParser::debug_print_block(t_block_directive b, std::string tabs) {
	std::cout << tabs << b.name << std::endl << tabs << "{" << std::endl;
	tabs = tabs + "   ";

	std::vector<t_simple_directive>::iterator it_s;
	for (it_s = b.simple_directives.begin(); it_s != b.simple_directives.end(); ++it_s)
		debug_print_simple(*it_s, tabs);

	std::vector<t_block_directive>::iterator it_b;
	for (it_b = b.block_directives.begin(); it_b != b.block_directives.end(); ++it_b)
		debug_print_block(*it_b, tabs);
	std::cout << tabs.substr(3) << "}" << std::endl;
}

void ConfigParser::debug_print_config() {
	std::cout << "DEBUG PRINTING CONFIG FILE THAT HAS BEEN PARSED:" << std::endl;
	debug_print_block(m_main_context, "");
}
