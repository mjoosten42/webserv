#include "Poller.hpp"

#include "FD.hpp"
#include "Server.hpp"
#include "SourceFds.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // pollfd

bool Poller::m_active = false;

void Poller::add(const Listener &listener) {
	int	   fd	  = listener.getFD();
	pollfd server = { fd, POLLIN, 0 };

	m_pollfds.push_back(server);
	m_listeners[fd] = listener;
}

void Poller::start() {
	m_active = true;

	for (auto &pair : m_listeners)
		pair.second.listen();

	LOG(GREEN "\n----STARTING LOOP----\n" DEFAULT);

	while (m_active) {
		LOG(CYAN << std::string(winSize(), '#') << DEFAULT);
		LOG(CYAN "Servers: " DEFAULT << rangeToString(m_pollfds.begin(), m_pollfds.begin() + clientsIndex()));
		LOG(CYAN "Clients: " DEFAULT << rangeToString(m_pollfds.begin() + clientsIndex(),
													  m_pollfds.begin() + responsesIndex()));
		LOG(CYAN "Sources: " DEFAULT << rangeToString(m_pollfds.begin() + responsesIndex(), m_pollfds.end()));

		int poll_status = WS::poll(m_pollfds);
		switch (poll_status) {
			case -1:
				if (errno == EINTR) // SIGCHLD
					continue;
				perror("poll");
			case 0: // Poll is blocking
				m_active = false;
				break;
			default:
				pollfdEvent();
		}
	}

	LOG(GREEN "\n----EXITING LOOP----" DEFAULT);
}

void Poller::quit() {
	m_active = false;
}

void Poller::pollfdEvent() {
	size_t i	   = 0; // Use an index because it can't be invalidated
	size_t servers = m_listeners.size();
	size_t clients = servers + m_connections.size();

	// Loop over the listening sockets for new clients
	for (; i != servers; i++) {
		pollfd &server = m_pollfds[i];

		if (server.revents) {
			LOG(CYAN "Server " DEFAULT << server.fd << CYAN " Set: " DEFAULT << getEventsAsString(server.events));
			LOG(CYAN "Server " DEFAULT << server.fd << CYAN " Get: " DEFAULT << getEventsAsString(server.revents));

			server.events = 0;

			if (server.revents & POLLIN)
				server.events |= acceptClient(server.fd);
		}
	}

	// loop over current clients to check if we can read or write
	for (; i != clients; i++) {
		pollfd	   &client = m_pollfds[i];
		Connection &conn   = m_connections[client.fd];

		if (client.revents) {
			LOG(CYAN "Client " DEFAULT << client.fd << CYAN " Set: " DEFAULT << getEventsAsString(client.events));
			LOG(CYAN "Client " DEFAULT << client.fd << CYAN " Get: " DEFAULT << getEventsAsString(client.revents));

			client.events = 0;

			if (client.revents & POLLIN && !(client.revents & POLLHUP))
				client.events |= conn.receive();

			if (client.revents & POLLOUT)
				client.events |= conn.send();

			if (client.revents & POLLHUP) {
				m_connections.erase(client.fd);
				m_toRemove.push_back(client.fd);
			}
		}
	}

	for (; i != m_pollfds.size(); i++) {
		pollfd	 &source   = m_pollfds[i];
		Response &response = *m_responses[source.fd];

		if (source.revents) {
			LOG(CYAN "Source " DEFAULT << source.fd << CYAN " Set: " DEFAULT << getEventsAsString(source.events));
			LOG(CYAN "Source " DEFAULT << source.fd << CYAN " Get: " DEFAULT << getEventsAsString(source.revents));

			source.events = 0;

			if (source.revents & POLLIN && !(source.revents & POLLHUP)) {
				source.events |= response.readFromCGI();

				for (auto &pair : m_connections)
					if (pair.second.getFirstReadFD() == source.fd)
						find(pair.second.getFD())->events |= POLLOUT;
			}

			if (source.revents & POLLOUT)
				source.events |= response.writeToCGI();

			if (source.revents & POLLHUP) {
				response.setDoneReading();
				removeSource(source.fd);
			}
		}
	}

	for (auto &client : m_newClients)
		m_pollfds.insert(m_pollfds.begin() + clientsIndex(), client);

	for (auto &source : m_newSources)
		m_pollfds.insert(m_pollfds.begin() + responsesIndex(), source);

	for (auto fd : m_toRemove)
		m_pollfds.erase(find(fd));

	m_newClients.clear();
	m_newSources.clear();
	m_toRemove.clear();
}

short Poller::acceptClient(int listener_fd) {
	const Listener &listener = m_listeners[listener_fd];
	FD				fd		 = ::accept(listener_fd, NULL, NULL);
	pollfd			client	 = { fd, POLLIN, 0 };

	if (fd == -1)
		perror("accept");
	else {
		set_fd_nonblocking(fd);

		m_newClients.push_back(client);
		m_connections[fd] = Connection(fd, &listener, this);

		LOG(CYAN "NEW CLIENT: " DEFAULT << fd);
	}

	return POLLIN;
}

void Poller::removeClient(int client_fd) {
	m_toRemove.push_back(client_fd); // erase from pollfd vector
	m_connections.erase(client_fd);	 // erase from connection map

	LOG(CYAN "CLIENT " DEFAULT << client_fd << CYAN " LEFT" DEFAULT);
}

void Poller::addSource(int fd, short flags, std::shared_ptr<Response> response) {
	pollfd source = { fd, flags, 0 };

	m_newSources.push_back(source);
	m_responses[fd] = response;
}

void Poller::removeSource(int fd) {
	m_toRemove.push_back(fd);
	m_responses.erase(fd);
}

size_t Poller::clientsIndex() {
	return m_listeners.size();
}

size_t Poller::responsesIndex() {
	return m_listeners.size() + m_connections.size();
}

std::vector<pollfd>::iterator Poller::find(int fd) {
	for (auto it = m_pollfds.begin(); it != m_pollfds.end(); it++)
		if (it->fd == fd)
			return it;
	LOG_ERR("Fd not found in pollfds: " << fd);
	LOG_ERR("Servers: " << rangeToString(m_pollfds.begin(), m_pollfds.end()));
	return m_pollfds.end();
}
