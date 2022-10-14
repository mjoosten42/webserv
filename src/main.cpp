#include "ConfigParser.hpp"
#include "Poller.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <vector>

int main(int argc, char **argv) {
	ConfigParser config;
	if (argc == 2)
		config.parse_config(argv[1]);
	else
		config.parse_config("nginx.conf");
	
	// EXAMPLE OF HOW TO FETCH BLOCK DIRECTIVES FROM THE CONFIG STRUCT
	std::vector<t_block_directive*> all_servers;
	all_servers = config.m_main_context.fetch_matching_blocks("server");
	int i = 0;
	std::vector<t_block_directive*>::iterator it;
	for (it = all_servers.begin(); it != all_servers.end(); ++it){
		++i;
		print("Server number " + toString(i) + ":");
		config.debug_print_block(**it, "");
	}

	std::vector<Server> servers;

	servers.push_back(8080);

	Poller poller(servers.begin(), servers.end());

	poller.start();
}
