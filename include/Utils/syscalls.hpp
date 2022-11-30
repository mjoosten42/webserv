#pragma once

#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

class EnvironmentMap;

namespace WS {

// Fds
int close(int fd);

// I.O.
ssize_t write(int fd, const std::string &str);

// Processes
pid_t fork();
pid_t wait();

// CGI
int chdir(const std::string &path);
int execve(const std::string &path, char *const argv[], const EnvironmentMap &em);

// Sockets
int poll(std::vector<pollfd> &pollfds);

// Paths
std::string realpath(const std::string &path);

} // namespace WS
