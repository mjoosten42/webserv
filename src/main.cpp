#include "Poller.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <vector>

int main() {
	std::vector<Server> servers;

	servers.push_back(8080);

	Poller poller(servers.begin(), servers.end());

	poller.start();
}
