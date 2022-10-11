#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//  http://nginx.org/en/docs/beginners_guide.html

//  simple_directives -> name and parameters separated by spaces and ends with a semicolon (;).
//  block_directives -> name and parameters separated by spaces and ends with a set of additional instructions
//  surrounded by braces ({ and }).

//  contexts -> directive that can have other directives inside braces, (examples:
//  events, http, server, and location).

//  main context -> Directives placed in the configuration file outside of any
//  contexts are considered to be in the main context.

typedef struct s_simple_directive {
		std::string name;
		std::string params;
} t_simple_directive;

typedef struct s_block_directive t_block_directive;

struct s_block_directive {
		std::string						name;
		std::vector<t_simple_directive> simple_directives;
		std::vector<t_block_directive>	block_directives;
		t_block_directive			   *parent_context;
};

class ConfigParser {
	public:
		ConfigParser();
		bool			  parse_config(const char *path);
		t_block_directive m_main_context;

	private: //  Reading file, checking validity
		std::vector<std::string> loadConfigToStrVector(const char *path);
		void					 discardComments(std::vector<std::string>					 &config);

	private: //	Finite state machine
		enum Token { SEMICOLON, COMMENT, OPEN_BRACE, CLOSE_BRACE, SIZE};
		char m_tokens[SIZE];
		void finite_state_machine(std::vector<std::string>& file);
		void state_simpledirective(t_block_directive **context, std::vector<std::string>::iterator it);
		void state_openblock(t_block_directive **context, std::vector<std::string>::iterator it);
		void state_closeblock(t_block_directive **context, std::vector<std::string>::iterator it);

	private: //  Debug functions
		void debug_print_simple(t_simple_directive s, std::string tabs);
		void debug_print_block(t_block_directive b, std::string tabs);
		void debug_print_config();
};
