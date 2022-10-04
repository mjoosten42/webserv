#include "Poller.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <vector>

int main() {
	std::vector<Server> servers;

	servers.push_back(Server());
	servers.push_back(Server());
	servers[0].setup(8080);
	servers[1].setup(8081);

	Poller poller(servers);
	poller.start();
}
