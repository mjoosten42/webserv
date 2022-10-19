#pragma once

#include <map>
#include <string>
#include <vector>

class Server;

// This class is basically a 'socket' class that can associate multiple servers with the same socket
class Listener {
	public:
		Listener();
		Listener(const std::string& listenAddress, short port);
		~Listener();

		void		  addServer(const Server *server);
		const Server *getServerByHost(const std::string& host) const;

		int				   getFD() const;
		short			   getPort() const;
		const std::string& getListenAddr() const;

	private:
		void setupSocket();

	private:
		int									  m_fd;
		std::string							  m_listenAddr; // aka Host
		short								  m_port;
		std::vector<const Server *>			  m_servers;
		std::map<std::string, const Server *> m_hostToServer;
};
