#include "ConfigParser.hpp"
#include "Listener.hpp"
#include "Poller.hpp"
#include "logger.hpp"
#include "syscalls.hpp"

#include <csignal>
#include <iostream>
#include <vector>

void signalHandler(int signum) {
	Poller ref;

	switch (signum) {
		case SIGINT: // Allow graceful exit
			ref.quit();
			break;
		case SIGPIPE: // CGI exited with write data remaining
			LOG("SIGPIPE");
			break;
		case SIGCHLD: // CGI exited
			WS::wait();
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

	// signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);
	signal(SIGCHLD, signalHandler);

	try {
		listeners = initFromConfig(argv[1]);

		for (auto& listener : listeners) {
			LOG(GREEN << listener.getListenerAsString("") << DEFAULT);
			poller.add(listener);
		}

		poller.start();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
