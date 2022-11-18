#pragma once

#include "Server.hpp"

#include <map>
#include <string>
#include <vector>

// This class is basically a 'socket' class that can associate multiple servers with the same socket
class Listener {
	public:
		Listener();
		Listener(const std::string& listenAddress, short port);

		void addServer(const Server& server);

		int				   getFD() const;
		short			   getPort() const;
		const Server	 & getServerByHost(const std::string	 &host) const;
		const std::string& getListenAddr() const;

		std::string getListenerAsString(std::string tabs) const;

	private:
		void setupSocket();

	private:
		int									  m_fd;
		std::string							  m_listenAddr; // aka Host
		short								  m_port;
		std::vector<Server>					  m_servers;
		std::map<std::string, const Server *> m_hostToServer;
};
