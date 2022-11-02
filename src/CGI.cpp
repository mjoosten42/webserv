#include "CGI.hpp"

#include "EnvironmentMap.hpp"
#include "MIME.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <fcntl.h>	  // open
#include <sys/wait.h> // waitpid

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
int Popen::my_popen(const std::string& path, const std::string& filename, const EnvironmentMap& em) {
	int serverToCgi[2];
	int cgiToServer[2];

	status = 502; // 502 bad gateway
	if (pipe(serverToCgi) == -1) {
		return 502;
	} else if (pipe(cgiToServer) == -1) {
		closePipe(serverToCgi);
		return status;
	}

	readfd	= cgiToServer[0];
	writefd = serverToCgi[1];

	set_fd_nonblocking(readfd);
	set_fd_nonblocking(writefd);

	pid = fork();
	switch (pid) {
		case -1: // failure
			closePipe(serverToCgi);
			closePipe(cgiToServer);
			return status;
		case 0: // child
			close(serverToCgi[1]);
			close(cgiToServer[0]);
			if (my_exec(serverToCgi[0], cgiToServer[1], path, filename, em.toCharpp()) == false)
				exit(EXIT_FAILURE);
			break;
		default: // parent
			close(serverToCgi[0]);
			close(cgiToServer[1]);
	}

	status = 200;
	return status;
}

// THIS SHOULD NEVER BE USED!
CGI& CGI::operator=(const CGI& other) {
	LOG_ERR("**** CGI = OPERATOR CALLED, SHOULD NOT BE CALLED!");
	popen = other.popen;
	exit(EXIT_FAILURE);
	return *this;
}

// returns the status code when the child CGI process has exited. returns -1 on failure or when it hasn't exited yet.
int CGI::didExit() {

	int stat_loc = 0;

	int status = waitpid(popen.pid, &stat_loc, WNOHANG);

	switch (status) {
		case -1:
			perror("waitpid");
		case 0:
			return -1;
		default:
			if (WIFEXITED(stat_loc))
				return WEXITSTATUS(stat_loc);
			break;
	}
	return 0;
}

// TODO: Set correct path
// TODO: when execution fails, close the pipe or something.
// TODO: gateway timeout
// TODO: throw instead of return?
int CGI::start(const Request& req, const Server *server, const std::string& command, const std::string& filename) {

	EnvironmentMap em;

	// check if file is openable beforehand.
	// if (access(filename.c_str(), F_OK)) {
	// 	if (errno == EACCES) {
	// 		return 403;
	// 	} else if (errno == ENOENT) {
	// 		return 404;
	// 	} else {
	// 		return 500;
	// 	}
	// }

	// TODO: make sure it is compliant https://en.wikipedia.org/wiki/Common_Gateway_Interface
	em["GATEWAY_INTERFACE"] = CGI_VERSION;
	em["SERVER_SOFTWARE"]	= server->getServerSoftwareName();
	em["SERVER_NAME"]		= req.getHost();
	em["SERVER_PORT"]	  = toString(server->getPort()); // TODO: when multiple ports, it should be the listener's port.
	em["SERVER_PROTOCOL"] = HTTP_VERSION;

	em["PATH_INFO"]	   = req.getLocation();
	em["QUERY_STRING"] = req.getQueryString();
	em["SCRIPT_NAME"]  = filename;

	// POST
	em["REQUEST_METHOD"] = req.getMethodAsString();
	em["CONTENT-LENGTH"] = req.getContentLength();
	em["CONTENT-TYPE"]	 = MIME::fromFileName(req.getLocation());

	return popen.my_popen(command, filename, em);
}
