#pragma once

#include "Connection.hpp"
#include "Listener.hpp"
#include "SourceFds.hpp"

#include <map>
#include <memory>	  // shared_ptr
#include <sys/poll.h> // pollf
#include <vector>

class Poller {
	public:
		void add(const Listener &listener);

		void start();
		void quit();

		void addSource(int fd, short flags, std::shared_ptr<Response> response);
		void removeSource(int fd);

		void pollfdEvent();

		short acceptClient(int listener_fd);
		void  removeClient(int client_fd);

		void addPollfds();

		size_t clientsIndex();
		size_t responsesIndex();

		std::vector<pollfd>::iterator find(int fd);

	private:
		static bool m_active;

		std::vector<pollfd> m_pollfds; // the array of pollfd structs

		std::map<FD, Listener>					 m_listeners;	// map server fd with corresponding listener
		std::map<FD, Connection>				 m_connections; // map client fd to its handler
		std::map<FD, std::shared_ptr<Response> > m_responses;	// map pipe fds to its response

		// Temporary additions/removals, to not mess up the pollfds loop
		std::vector<pollfd> m_newClients;
		std::vector<pollfd> m_newSources;
		std::vector<int>	m_toRemove;
};
