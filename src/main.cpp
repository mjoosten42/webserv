#include "ConfigParser.hpp"
#include "Poller.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include "syscalls.hpp"

#include <csignal>
#include <iostream>
#include <vector>

// TODO: remove
#include <sys/resource.h> // setrlimit

void limit_max_open(rlim_t limit) {
	struct rlimit rlim = { limit, limit };

	if (setrlimit(RLIMIT_NOFILE, &rlim) == -1)
		perror("setrlimit");
}

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

	// signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);
	signal(SIGCHLD, signalHandler);

	for (auto& listener : listeners)
		poller.add(listener);

	limit_max_open(200);

	poller.start();
}
