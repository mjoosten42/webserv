#pragma once

#include <string>
#include <unistd.h> // pid_t, close, pipe, fork etc.

class Response;
class EnvironmentMap;

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
		CGI(Response& response);
		~CGI();
		CGI& operator=(const CGI& other);

		int start(const std::string& command, const std::string& filename);
		// TODO: how to close readfd?

	public:
		Popen popen;

	private:
		Response& m_response;
};
