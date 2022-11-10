#include "ConfigParser.hpp"
#include "Listener.hpp"
#include "Server.hpp"
#include "logger.hpp"

#include <string>
#include <vector>

static std::vector<Server> initServers(ConfigParser& config) {
	std::vector<Server>				 servers;
	std::vector<t_block_directive *> server_config_blocks = config.m_main_context.fetch_matching_blocks("server");
	std::vector<t_block_directive *>::iterator it		  = server_config_blocks.begin();

	for (; it != server_config_blocks.end(); ++it) {
		servers.push_back(Server(*it));
		// config.debug_print_block(**it, ""); // Can remove this and make it a one line loop.
	}

	return servers;
}

static Listener *getListenerByHostPort(std::vector<Listener>& listeners, const std::string& host, const short port) {

	for (std::vector<Listener>::iterator it = listeners.begin(); it != listeners.end(); it++)
		if (it->getListenAddr() == host && it->getPort() == port)
			return &*it;
	return NULL;
}

std::vector<Listener> initFromConfig(const char *file) {
	std::vector<Listener> listeners;
	ConfigParser		  config(file);
	std::vector<Server>	  servers = initServers(config);

	LOG(config.getConfigAsString());

	for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); it++) {

		Listener *listener = getListenerByHostPort(listeners, it->getHost(), it->getPort());

		if (!listener) {
			listeners.push_back(Listener(it->getHost(), it->getPort()));
			listener = &listeners.back();
		}

		listener->addServer(*it);
	}

	return listeners;
}
