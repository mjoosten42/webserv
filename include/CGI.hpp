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

		int my_popen(const std::string& path, const std::string& filename, const EnvironmentMap& em);

		void closeFDs() {
			close(readfd);
			close(writefd);
		}
};

class CGI {
	public:
		CGI& operator=(const CGI& other);

		int didExit();

		int start(const Request	   & request,
				  const Server		*server,
				  const std::string& command,
				  const std::string& filename);
		// TODO: how to close readfd?

	public:
		Popen popen;
};
