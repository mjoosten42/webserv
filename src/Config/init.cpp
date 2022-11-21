#include "ConfigParser.hpp"
#include "Listener.hpp"
#include "Server.hpp"
#include "logger.hpp"

#include <string>
#include <vector>

static std::vector<Server> initServers(ConfigParser &config) {
	std::vector<t_block_directive *> server_config_blocks = config.m_main_context.fetch_matching_blocks("server");
	std::vector<Server>				 servers(server_config_blocks.size()); // Default construct all servers

	for (size_t i = 0; i != servers.size(); i++)
		servers[i].add(server_config_blocks[i]);

	return servers;
}

static Listener *getListenerByHostPort(std::vector<Listener> &listeners, const std::string &host, const short port) {

	for (auto &listener : listeners)
		if (listener.getListenAddr() == host && listener.getPort() == port)
			return &listener;
	return NULL;
}

// Get all servers from config, then move overlapping hosts/ports to the same listener
std::vector<Listener> initFromConfig(const char *file) {
	ConfigParser		  config(file);
	std::vector<Listener> listeners;
	std::vector<Server>	  servers = initServers(config);

	for (auto &server : servers) {
		Listener *listener = getListenerByHostPort(listeners, server.getHost(), server.getPort());

		if (!listener) {
			listeners.push_back(Listener(server.getHost(), server.getPort()));
			listener = &listeners.back();
		}

		listener->addServer(server);
	}

	return listeners;
}
