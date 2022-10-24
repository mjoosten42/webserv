#include "CGI.hpp"

#include "EnvironmentMap.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "utils.hpp"

static void closePipe(int *pfds) {
	close(pfds[0]);
	close(pfds[1]);
}

static bool my_exec(int infd, int outfd, const std::string& path, const std::string& filename, char **envp) {
	close(STDIN_FILENO);
	if (dup(infd) == -1)
		return false;
	close(infd);

	close(STDOUT_FILENO);
	if (dup(outfd) == -1)
		return false;
	close(outfd);

	const char *args[] = { path.c_str(), filename.c_str(), NULL };

	execve(path.c_str(), const_cast<char *const *>(args), const_cast<char *const *>(envp));
	return false;
}

// back to minishell ayy
// TODO: gateway timeout
int Popen::my_popen(const std::string& path, const std::string& filename, const EnvironmentMap& em) {
	int serverToCgi[2];
	int cgiToServer[2];

	status = 502; // 502 bad gateway
	if (pipe(serverToCgi) == -1) {
		return status;
	} else if (pipe(cgiToServer) == -1) {
		closePipe(serverToCgi);
		return status;
	}

	readfd	= cgiToServer[0];
	writefd = serverToCgi[1];

	pid = fork();
	if (pid == -1) {
		closePipe(serverToCgi);
		closePipe(cgiToServer);
		return status;
	} else if (pid == 0) {
		close(serverToCgi[1]);
		close(cgiToServer[0]);
		if (my_exec(serverToCgi[0], cgiToServer[1], path, filename, em.toCharpp()) == false)
			exit(EXIT_FAILURE);
	} else {
		close(serverToCgi[0]);
		close(cgiToServer[1]);
	}

	status = 200;
	return status;
}

CGI::CGI() {}

CGI::~CGI() {}

#include <iostream>

// THIS SHOULD NEVER BE USED!
CGI& CGI::operator=(const CGI& other) {
	std::cerr << "**** CGI = OPERATOR CALLED, SHOULD NOT BE CALLED!\n";
	popen = other.popen;
	exit(EXIT_FAILURE);
	return *this;
}

// TODO: Set correct path
//  TODO: handle like a "downloaded" file
int CGI::start(const Request& req, const Server *server, const std::string& command, const std::string& filename) {

	EnvironmentMap em;
	em.initFromEnviron();

	//  TODO: make sure it is compliant https://en.wikipedia.org/wiki/Common_Gateway_Interface
	em["SERVER_SOFTWARE"] = server->getName();
	em["SERVER_NAME"]	  = req.getHost();
	em["REQUEST_METHOD"]  = req.getMethodAsString();
	em["PATH_INFO"]		  = req.getLocation();
	em["QUERY_STRING"]	  = req.getQueryString();

	return popen.my_popen(command, filename, em);
}
