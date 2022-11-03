#include "CGI.hpp"

#include "EnvironmentMap.hpp"
#include "MIME.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "buffer.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <fcntl.h>	  // open
#include <sys/wait.h> // waitpid

static void closePipe(int pfds[2]) {
	close(pfds[0]);
	close(pfds[1]);
}

static bool my_exec(int infd, int outfd, const std::string& filename, char *const envp[]) {
	if (dup2(infd, STDIN_FILENO) == -1)
		return false;
	close(infd);

	if (dup2(outfd, STDOUT_FILENO) == -1)
		return false;
	close(outfd);

	getcwd(path, PATH_MAX);
	std::string str = path;
	str += "/" + filename;
	str.erase(str.find_last_of('/'));
	if (chdir(str.c_str()) == -1)
		perror("chdir");

	std::string copy   = filename.substr(filename.find_last_of('/') + 1);
	char *const args[] = { const_cast<char *const>(copy.c_str()), NULL };

	execve(copy.c_str(), args, const_cast<char *const *>(envp));
	perror("execve"); // TODO
	return false;
}

// back to minishell ayy
int Popen::my_popen(const std::string& filename, const EnvironmentMap& em) {
	int serverToCgi[2];
	int cgiToServer[2];

	if (pipe(serverToCgi) == -1) {
		return 502;
	} else if (pipe(cgiToServer) == -1) {
		closePipe(serverToCgi);
		return 502;
	}

	readfd	= cgiToServer[0];
	writefd = serverToCgi[1];

	set_fd_nonblocking(readfd);
	// set_fd_nonblocking(writefd);

	pid = fork();
	switch (pid) {
		case -1: // failure
			closePipe(serverToCgi);
			closePipe(cgiToServer);
			return status;
		case 0: // child
			close(serverToCgi[1]);
			close(cgiToServer[0]);
			if (my_exec(serverToCgi[0], cgiToServer[1], filename, em.toCharpp()) == false)
				exit(EXIT_FAILURE);
			break;
		default: // parent
			close(serverToCgi[0]);
			close(cgiToServer[1]);
	}

	return 200;
}

// returns the status code when the child CGI process has exited. returns -1 on failure or when it hasn't exited yet.
bool CGI::didExit() {

	int	 stat_loc = 0;
	int	 status	  = waitpid(popen.pid, &stat_loc, WNOHANG);
	bool exit	  = false;

	switch (status) {
		case -1:
			perror("waitpid");
		case 0:
			return -1;
		default:
			exit = WIFEXITED(stat_loc);
	}
	return exit;
}

// TODO: Set correct path
// TODO: when execution fails, close the pipe or something.
// TODO: gateway timeout
// TODO: throw instead of return?
int CGI::start(const Request& req, const Server *server, const std::string& filename) {

	EnvironmentMap em;

	// TODO: make sure it is compliant https://en.wikipedia.org/wiki/Common_Gateway_Interface
	em["GATEWAY_INTERFACE"] = CGI_VERSION;
	em["SERVER_SOFTWARE"]	= server->getServerSoftwareName();
	em["SERVER_NAME"]		= req.getHost();
	em["SERVER_PORT"]	  = toString(server->getPort()); // TODO: when multiple ports, it should be the listener's port.
	em["SERVER_PROTOCOL"] = HTTP_VERSION;

	em["PATH_INFO"]		  = req.getLocation();
	em["PATH_TRANSLATED"] = req.getLocation();
	em["QUERY_STRING"]	  = req.getQueryString();
	em["SCRIPT_NAME"]	  = filename;

	// POST
	em["REQUEST_METHOD"] = req.getMethodAsString();
	em["CONTENT_LENGTH"] = toString(req.getContentLength());
	em["CONTENT_TYPE"]	 = req.getHeaderValue("Content-Type");

	em["UPLOAD_DIR"] = getRealPath("html") + "/uploads/";

	return popen.my_popen(filename, em);
}
