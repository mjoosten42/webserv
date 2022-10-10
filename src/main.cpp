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

	std::vector<Server> servers;

	servers.push_back(8080);

	Poller poller(servers.begin(), servers.end());

	poller.start();
}
