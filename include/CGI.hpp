#pragma once

#include <string>
#include <unistd.h> // pid_t, close, pipe, fork etc.

class EnvironmentMap;
class Request;
class Server;

struct Popen {
		pid_t pid;
		int	  readfd;
		int	  writefd;
		int	  status;

		void my_popen(const std::string& filename, const EnvironmentMap& em);

		void closeFDs();
};

class CGI {
	public:
		void start(const Request& request, const Server *server, const std::string& filename, const std::string& peer);

	public:
		Popen popen;
};
