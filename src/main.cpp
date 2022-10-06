#include "Poller.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <vector>

int main() {
	const Server *servers = new Server(8080);
	Poller		  poller(servers, 1);

	poller.start();

	//  Never reached
	delete servers;
}
