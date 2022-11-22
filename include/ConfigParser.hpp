#pragma once

#include <string>
#include <vector>

// http://nginx.org/en/docs/beginners_guide.html

// simple_directives -> name and parameters separated by spaces and ends with a semicolon (;).
// block_directives -> name and parameters separated by spaces and ends with a set of additional instructions
// surrounded by braces ({ and }).

// contexts -> directive that can have other directives inside braces, (examples:
// events, http, server, and location).

// main context -> Directives placed in the configuration file outside of any
// contexts are considered to be in the main context.

class Listener;

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

		std::vector<t_block_directive *> fetch_matching_blocks(const std::string &blocks_to_fetch);

		std::string fetch_simple(const std::string &key);

	private:
		void recurse_blocks(std::vector<t_block_directive *> &ret, const std::string &blocks_to_fetch);
};

class ConfigParser {
	public:
		ConfigParser(const char *path);
		std::vector<std::string> readFile(const char *path);

	public:
		t_block_directive m_main_context;

	private: // Checking validity
		void check_validity(std::vector<std::string> &config);
		void discard_comments(std::vector<std::string> &config);
		void discard_empty(std::vector<std::string> &config);
		void check_braces_error(std::vector<std::string> &config);
		void check_semicolon_error(std::vector<std::string> &config);
		void check_overflow_errors(std::vector<std::string>			  &config,
								   std::vector<std::string>::iterator &file_it,
								   const t_simple_directive			  &to_check);
		void throw_config_error(std::vector<std::string>		   &config,
								std::vector<std::string>::iterator &file_it,
								const std::string				   &reason);

	private: //	Finite state machine
		void finite_state_machine(std::vector<std::string> &file);
		void state_openblock(t_block_directive **context, std::vector<std::string>::iterator &it);
		void state_closeblock(t_block_directive **context);
		void state_simpledirective(t_block_directive				 **context,
								   std::vector<std::string>::iterator &it,
								   std::vector<std::string>			  &file);

		enum Token { SEMICOLON, COMMENT, OPEN_BRACE, CLOSE_BRACE, SIZE };

		char m_tokens[SIZE];

	public: // Debug printing
		std::string getConfigAsString();
		std::string getSimpleAsString(const t_simple_directive &s, const std::string &tabs);
		std::string getBlockAsString(const t_block_directive &b, const std::string &tabs);
};

// Initialising of Listeners based on config:
std::vector<Listener> initFromConfig(const char *path);
