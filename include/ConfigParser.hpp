#pragma once

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
		std::string						additional_params;
		std::vector<t_simple_directive> simple_directives;
		std::vector<t_block_directive>	block_directives;
		t_block_directive			   *parent_context;
		std::vector<t_block_directive*> fetch_matching_blocks(std::string blocks_to_fetch);
		private:
		void recurse_blocks(std::vector<t_block_directive*>& ret, std::string blocks_to_fetch);
};

class ConfigParser {
	public:
		ConfigParser();
		bool					 parse_config(const char *path);
		std::vector<std::string> readFile(const char *path);

	public:
		t_block_directive m_main_context;

	private: //  Checking validity
		void check_validity(std::vector<std::string>& config);
		void discard_comments(std::vector<std::string>& config);
		void check_braces_error(std::vector<std::string>& config);
		void check_semicolon_error(std::vector<std::string>& config);
		void throw_config_error(std::vector<std::string>		  & config,
								std::vector<std::string>::iterator& file_it,
								std::string							reason);

	private: //	Finite state machine
		void finite_state_machine(std::vector<std::string>& file);
		void state_openblock(t_block_directive **context, std::vector<std::string>::iterator& it);
		void state_closeblock(t_block_directive **context);
		void state_simpledirective(t_block_directive				 **context,
								   std::vector<std::string>::iterator& it,
								   std::vector<std::string>			 & file);

		enum Token { SEMICOLON, COMMENT, OPEN_BRACE, CLOSE_BRACE, SIZE };

		char m_tokens[SIZE];

	public: //  Debug printing
		void debug_print();
		void debug_print_simple(t_simple_directive s, std::string tabs);
		void debug_print_block(t_block_directive b, std::string tabs);
};
