#include "ConfigParser.hpp"
#include "Poller.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <vector>

void launchServers(std::vector<Server>& servers, ConfigParser& config) {
	std::vector<t_block_directive *> server_config_blocks;
	server_config_blocks = config.m_main_context.fetch_matching_blocks("server");
	std::vector<t_block_directive *>::iterator it;
	for (it = server_config_blocks.begin(); it != server_config_blocks.end(); ++it) {
		std::string listen_port = (**it).fetch_simple("listen");
		if (!listen_port.empty())
			servers.push_back(stoi(listen_port));
		else
			print("FAILED TO START, NO LISTEN PORT SPECIFIED");
		config.debug_print_block(**it, "");
	}
	return;
}

int main(int argc, char *argv[]) {
	ConfigParser config;
	if (argc == 2)
		config.parse_config(argv[1]);
	else
		config.parse_config("default.conf");
	std::vector<Server> servers;
	launchServers(servers, config); //  Will be moved somewhere, don't know where would be nicest yet.

	Poller poller(servers.begin(), servers.end());

	poller.start();
}
