#include "CGI.hpp"

#include "EnvironmentMap.hpp"
#include "MIME.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "buffer.hpp"
#include "logger.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

#include <fcntl.h>		// open
#include <sys/socket.h> // setsockopt
#include <sys/wait.h>	// waitpid

static void closePipe(int pfds[2]) {
	WS::close(pfds[0]);
	WS::close(pfds[1]);
}

static void pipeTwo(int serverToCgi[2], int CgiToServer[2]) {
	if (pipe(serverToCgi) == -1)
		throw 502;
	if (pipe(CgiToServer) == -1) {
		closePipe(serverToCgi);
		throw 502;
	}
}

static void dupTwo(int in_fd, int out_fd) {
	if (dup2(in_fd, STDIN_FILENO) == -1)
		exit(EXIT_FAILURE);
	WS::close(in_fd);

	if (dup2(out_fd, STDOUT_FILENO) == -1) {
		WS::close(STDIN_FILENO);
		throw 502;
	}
	WS::close(out_fd);
}

static void my_exec(int infd, int outfd, const std::string& filename, const EnvironmentMap& em) {
	dupTwo(infd, outfd);

	WS::chdir(filename.substr(0, filename.find_last_of("/")));

	std::string copy   = filename.substr(filename.find_last_of("/") + 1);
	char *const args[] = { const_cast<char *const>(copy.c_str()), NULL };

	WS::execve(copy, args, em);
	exit(EXIT_FAILURE);
}

void Popen::closeFDs() {
	WS::close(readfd);
	WS::close(writefd);
}

// back to minishell ayy
void Popen::my_popen(const std::string& filename, const EnvironmentMap& em) {
	int serverToCgi[2];
	int cgiToServer[2];

	pipeTwo(serverToCgi, cgiToServer);

	readfd	= cgiToServer[0];
	writefd = serverToCgi[1];

	set_fd_nonblocking(readfd);
	set_fd_nonblocking(writefd); // TODO

	pid = WS::fork();
	switch (pid) {
		case -1: // failure
			closePipe(serverToCgi);
			closePipe(cgiToServer);
			throw 502;
		case 0: // child
			WS::close(serverToCgi[1]);
			WS::close(cgiToServer[0]);
			my_exec(serverToCgi[0], cgiToServer[1], filename, em);
		default: // parent
			WS::close(serverToCgi[0]);
			WS::close(cgiToServer[1]);
	}
}

// TODO: Set correct path
// TODO: when execution fails, close the pipe or something.
void CGI::start(const Request& req, const Server *server, const std::string& filename) {

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

	em["UPLOAD_DIR"] = WS::realpath("html") + "/uploads/";

	popen.my_popen(filename, em);
}
