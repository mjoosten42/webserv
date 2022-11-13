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

	if (WS::chdir(filename.substr(0, filename.find_last_of("/"))) == -1)
		exit(EXIT_FAILURE);

	std::string copy   = filename.substr(filename.find_last_of("/") + 1);
	char *const args[] = { const_cast<char *const>(copy.c_str()), NULL }; //Werror type qualifiers ignored on cast result type

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
			break;
		default: // parent
			WS::close(serverToCgi[0]);
			WS::close(cgiToServer[1]);
	}
}

// https://www.rfc-editor.org/rfc/rfc3875#section-4.1.5

// TODO: Set correct path
void CGI::start(const Request& req, const Server *server, const std::string& filename, const std::string& peer) {

	EnvironmentMap em;

	// Unused
	// em["AUTH_TYPE"];
	// em["REMOTE_IDENT"];
	// em["REMOTE_USER"];

	if (req.getContentLength() > 0)
		em["CONTENT_LENGTH"] = toString(req.getContentLength());

	if (req.hasHeader("Content-Type"))
		em["CONTENT_TYPE"] = req.getHeaderValue("Content-Type");

	em["PATH_INFO"]		  = req.getPathInfo(); // TODO
	em["PATH_TRANSLATED"] = "";				   // TODO

	em["QUERY_STRING"]	 = req.getQueryString();
	em["REMOTE_ADDR"]	 = peer;
	em["REMOTE_HOST"]	 = peer;
	em["REQUEST_METHOD"] = req.getMethodAsString();
	em["SCRIPT_NAME"]	 = filename;
	em["SERVER_NAME"]	 = req.getHost();

	// TODO: when multiple ports, it should be the listener's port.
	// M: an incoming connection can only have one port right?
	em["SERVER_PORT"] = toString(server->getPort());

	if (req.getMethod() == POST)
		em["UPLOAD_DIR"] = WS::realpath("html") + "/uploads/"; // TODO

	LOG("PATH_INFO: " << em["PATH_INFO"]);
	LOG("PATH_TRANSLATED: " << em["PATH_TRANSLATED"]);

	popen.my_popen(filename, em);
}
