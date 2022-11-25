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

struct simple_directive {
		std::string name;
		std::string params;
};

struct block_directive {	
		std::string					  name;
		std::string					  additional_params;
		std::vector<simple_directive> simple_directives;
		std::vector<block_directive>  block_directives;
		struct block_directive		 *parent_context;

		std::vector<struct block_directive *> fetch_matching_blocks(const std::string &blocks_to_fetch);

		std::string fetch_simple(const std::string &key);

	private:
		void recurse_blocks(std::vector<struct block_directive *> &ret, const std::string &blocks_to_fetch);
};

class ConfigParser {
	public:
		ConfigParser(const char *path);
		ConfigParser(const std::string& data); // Fuzzing

	public:
		block_directive m_main_context;

	private:
		void readFile(const char *path);

		// Checking validity
		void check_validity();
		void discard_comments();
		void discard_empty();
		void check_braces_error();
		void check_semicolon_error();
		void check_overflow_errors(size_t line, const simple_directive &to_check);
		void throw_config_error(size_t line, const std::string &reason);

		//	Finite state machine
		void finite_state_machine();
		void state_simpledirective(block_directive **context, const std::string &line, size_t i);
		void state_openblock(block_directive **context, const std::string &line);
		void state_closeblock(block_directive **context);

		enum Token { SEMICOLON, COMMENT, OPEN_BRACE, CLOSE_BRACE, SIZE };

		static char m_tokens[SIZE];

		std::vector<std::string> config;

	public: // Debug printing
		std::string getConfigAsString();
		std::string getSimpleAsString(const simple_directive &s, const std::string &tabs);
		std::string getBlockAsString(const block_directive &b, const std::string &tabs);
};

// Initialising of Listeners based on config:
std::vector<Listener> initFromConfig(const char *path);
std::vector<Listener> initFromConfig(const std::string& data); // Fuzzer
