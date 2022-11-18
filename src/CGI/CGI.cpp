#include "CGI.hpp"

#include "EnvironmentMap.hpp"
#include "MIME.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "buffer.hpp"
#include "logger.hpp"
#include "methods.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

#include <sys/socket.h> // setsockopt
#include <sys/wait.h>	// waitpid
#include <unistd.h>		// fork

static void closePipe(int pfds[2]) {
	WS::close(pfds[0]);
	WS::close(pfds[1]);
}

static void pipeTwo(int serverToCgi[2], int CgiToServer[2]) {
	int ret[2] = { pipe(serverToCgi), pipe(CgiToServer) };

	if (ret[0] == -1 || ret[1] == -1) {
		if (ret[0] != -1)
			closePipe(serverToCgi);
		if (ret[1] != -1)
			closePipe(CgiToServer);
		if (errno == EMFILE)
			throw 503;
		throw 500;
	}
}

static void dupTwo(int in_fd, int out_fd) {
	if (dup2(in_fd, STDIN_FILENO) == -1)
		exit(EXIT_FAILURE);
	WS::close(in_fd);

	if (dup2(out_fd, STDOUT_FILENO) == -1) {
		WS::close(STDIN_FILENO);
		exit(EXIT_FAILURE);
	}
	WS::close(out_fd);
}

static void my_exec(int infd, int outfd, const std::string& filename, const EnvironmentMap& em) {
	dupTwo(infd, outfd);

	if (WS::chdir(filename.substr(0, filename.find_last_of("/"))) == -1)
		exit(EXIT_FAILURE);

	std::string copy   = filename.substr(filename.find_last_of("/") + 1);
	char *const args[] = { const_cast<char *const>(copy.c_str()),
						   NULL }; // Werror type qualifiers ignored on cast result type

	WS::execve(copy, args, em);
	exit(EXIT_FAILURE);
}

// back to minishell ayy
void Popen::my_popen(const std::string& filename, const EnvironmentMap& em) {
	int serverToCgi[2];
	int cgiToServer[2];

	pipeTwo(serverToCgi, cgiToServer);

	pid = fork();
	switch (pid) {
		case -1: // failure
			perror("fork");
			closePipe(serverToCgi);
			closePipe(cgiToServer);
			if (errno == EAGAIN)
				throw 503;
			throw 500;
		case 0: // child
			WS::close(serverToCgi[1]);
			WS::close(cgiToServer[0]);
			my_exec(serverToCgi[0], cgiToServer[1], filename, em);
			break;
		default: // parent
			WS::close(cgiToServer[1]);
			WS::close(serverToCgi[0]);

			WS::fcntl(cgiToServer[0]);
			WS::fcntl(serverToCgi[1]);

			readfd	= cgiToServer[0];
			writefd = serverToCgi[1];
	}
}

// https://www.rfc-editor.org/rfc/rfc3875#section-4.1.5

void CGI::start(const Response& response) {
	const Request& req = response.m_request;
	EnvironmentMap em;

	em.addEnv();

	em["GATEWAY_INTERFACE"] = CGI_VERSION;
	em["SERVER_PROTOCOL"]	= HTTP_VERSION;
	em["SERVER_SOFTWARE"]	= SERVER_SOFTWARE;

	// Unused
	// em["AUTH_TYPE"];
	// em["REMOTE_IDENT"];
	// em["REMOTE_USER"];

	if (req.getContentLength() > 0)
		em["CONTENT_LENGTH"] = toString(req.getContentLength());

	if (req.hasHeader("Content-Type"))
		em["CONTENT_TYPE"] = req.getHeaderValue("Content-Type");

	em["PATH_INFO"]		  = req.getPathInfo();
	em["PATH_TRANSLATED"] = req.getPathInfo();

	em["QUERY_STRING"]	 = req.getQueryString();
	em["REMOTE_ADDR"]	 = response.m_peer;
	em["REMOTE_HOST"]	 = response.m_peer;
	em["REQUEST_METHOD"] = toString(req.getMethod());
	em["SCRIPT_NAME"]	 = response.m_filename;
	em["SERVER_NAME"]	 = req.getHost();
	em["SERVER_PORT"]	 = toString(response.m_server->getPort());

	if (req.getMethod() == POST) {
		std::string root = response.m_server->getRoot(response.m_locationIndex);
		std::string dir	 = WS::realpath(root) + response.m_server->getUploadDir(response.m_locationIndex);
	
		em["UPLOAD_DIR"] = dir;
		LOG("Upload directory: " << dir);
	}

	popen.my_popen(response.m_filename, em);
}
