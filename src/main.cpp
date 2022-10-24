#include "ConfigParser.hpp"
#include "Poller.hpp"
#include "Server.hpp"
// #include "buffer.hpp"
// #include "defines.hpp"
// #include "utils.hpp"

#include <vector>

int main(int argc, char *argv[]) {
	ConfigParser config;
	if (argc == 2)
		config.parse_config(argv[1]);
	else
		config.parse_config("default.conf");

	std::vector<Listener> listeners;
	initFromConfig(config, listeners);

	Poller poller(listeners.begin(), listeners.end());

	poller.start();
}
