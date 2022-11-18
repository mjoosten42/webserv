#pragma once

#include "FD.hpp"

#include <string>
#include <unistd.h> // pid_t

class EnvironmentMap;
class Response;

struct Popen {
		pid_t pid;
		FD	  readfd;
		FD	  writefd;

		void my_popen(const std::string& filename, const EnvironmentMap& em);
};

class CGI {
	public:
		void start(const Response& response);

	public:
		Popen popen;
};
