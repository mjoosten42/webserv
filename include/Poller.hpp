#pragma once

#include <vector>

#include <poll.h>

typedef struct pollfd pollfd_t;

class Poller
{
public:
	Poller(int server_socket);
	~Poller();

	void start(); // starts the polling.

private:
	pollfd_t create_pollfd(int fd, short events) const;

	void acceptClient();
	bool receiveFromClient(int fd);

private:
	int m_servsock; // fd from the listening server socket
	std::vector<pollfd_t> m_pollfds; // the array of pollfd structs
};
