#include "ConfigParser.hpp"
#include "Poller.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include <iostream>

#include <vector>

int main(int argc, const char *argv[]) {
	switch (argc) {
		case 1:
			argv[1] = "default.conf";
		case 2:
			break;
		default:
			std::cerr << "usage: ./webserv [configuration file]\n";
			return EXIT_FAILURE;
	}

	ConfigParser config;
	try {
		config.parse_config(argv[1]);
	} catch(...) {
		return EXIT_FAILURE;
	}

	LOG(config.getConfigAsString());

	std::vector<Listener> listeners;
	initFromConfig(config, listeners);

	// DEBUG TESTS:
	std::string test = listeners.front().getServerByHost("amogus.localhst.co.uk").getRootForFile("amogus.jpg");
	LOG("Result:");
	LOG(test);

	Poller poller(listeners.begin(), listeners.end());
	poller.start();
}
