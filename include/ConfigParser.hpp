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
};

class ConfigParser {
	public:
		t_block_directive main_context;
};
