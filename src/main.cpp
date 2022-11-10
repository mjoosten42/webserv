#include "ConfigParser.hpp"
#include "Poller.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include "syscalls.hpp"

#include <iostream>
#include <vector>

void signalHandler(int signum) {
	Poller ref;
	pid_t  child;

	switch (signum) {
		case SIGINT: // Allow gracefull exit
			ref.quit();
			break;
		case SIGPIPE: // CGI exited with write data remaining
			LOG("SIGPIPE");
			break;
		case SIGCHLD: // CGI exited
			child = WS::wait();
			LOG(RED "Reaped child: " DEFAULT << child);
	}
}

int main(int argc, const char *argv[]) {
	std::vector<Listener> listeners;
	Poller				  poller;

	switch (argc) {
		case 1:
			argv[1] = "default.conf";
		case 2:
			break;
		default:
			std::cerr << "Usage: ./webserv [configuration file]\n";
			return EXIT_FAILURE;
	}

	try {
		listeners = initFromConfig(argv[1]);
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);
	signal(SIGCHLD, signalHandler);

	for (size_t i = 0; i < listeners.size(); i++)
		poller.add(listeners[i]);

	poller.start();
}
