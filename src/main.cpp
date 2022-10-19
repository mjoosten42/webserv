#include "ConfigParser.hpp"
#include "Poller.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <vector>

static std::vector<Server> initServers(ConfigParser& config) {
	std::vector<Server>				 servers;
	std::vector<t_block_directive *> server_config_blocks = config.m_main_context.fetch_matching_blocks("server");
	std::vector<t_block_directive *>::iterator it		  = server_config_blocks.begin();

	for (; it != server_config_blocks.end(); ++it) {
		servers.push_back(Server(*it));
		config.debug_print_block(**it, "");
	}

	return servers;
}

static Listener *getListenerByHostPort(std::vector<Listener>& listeners, const std::string& host, const short port) {

	for (std::vector<Listener>::iterator it = listeners.begin(); it != listeners.end(); it++)
		if (it->getListenAddr() == host && it->getPort() == port)
			return &*it;
	return NULL;
}

static void initListeners(std::vector<Server>& servers, std::vector<Listener>& listeners) {

	for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); it++) {

		Listener *listener = getListenerByHostPort(listeners, it->getHost(), it->getPort());

		if (!listener) {
			listeners.push_back(Listener(it->getHost(), it->getPort()));
			listener = &listeners.back();
		}

		listener->addServer(&*it);
	}
}

int main(int argc, char *argv[]) {
	ConfigParser config;
	if (argc == 2)
		config.parse_config(argv[1]);
	else
		config.parse_config("default.conf");
	std::vector<Server> servers =
		initServers(config); //  Will be moved somewhere, don't know where would be nicest yet.

	std::vector<Listener> listeners;
	initListeners(servers, listeners);

	Poller poller(listeners.begin(), listeners.end());

	poller.start();
}
