#include "Handler.hpp"
#include "Request.hpp"
#include "utils.hpp"
#include "EnvironmentMap.hpp"

struct Popen {
		pid_t pid;
		int	  readfd;
		int	  writefd;
		int	  status;

		void closeFDs() {
			close(readfd);
			close(writefd);
		}
};

static void closePipe(int *pfds)
{
	close(pfds[0]);
	close(pfds[1]);
}

bool my_exec(int infd, int outfd, const std::string& path, const std::string& filename, char **envp)
{
	close(STDIN_FILENO);
	if (dup(infd) == -1)
		return false;
	close(infd);

	close(STDOUT_FILENO);
	if (dup(outfd) == -1)
		return false;
	close(outfd);

	const char *args[] = { path.c_str(), filename.c_str(), nullptr };

	if (execve(path.c_str(), const_cast<char *const *>(args), const_cast<char *const *>(envp)) == -1)
		return false;
	return true; //  lol never reached
}


Popen my_popen(const std::string& path, const std::string& filename, const EnvironmentMap& em)
{
	Popen popen;
	int	  serverToCgi[2];
	int	  cgiToServer[2];

	popen.status = 500;
	if (pipe(serverToCgi) == -1) {
		return popen;
	}
	else if (pipe(cgiToServer) == -1)
	{
		closePipe(serverToCgi);
		return popen;
	}

	popen.readfd  = cgiToServer[0];
	popen.writefd = serverToCgi[1];

	popen.pid = fork();
	if (popen.pid == -1)
	{
		closePipe(serverToCgi);
		closePipe(cgiToServer);
		return popen;
	} else if (popen.pid == 0) {
		close(serverToCgi[1]);
		close(cgiToServer[0]);
		if (my_exec(serverToCgi[0], cgiToServer[1], path, filename, em.toCharpp()) == false) {
			exit(EXIT_FAILURE);
		}
	} else {
		close(serverToCgi[0]);
		close(cgiToServer[1]);
	}

	popen.status = 200;
	return popen;
}

// TODO: handle like a "downloaded" file
int Handler::handleCGI(const std::string& command, const std::string& filename)
{
	EnvironmentMap em;
	em.initFromEnviron();

	// TODO: make sure it is compliant https://en.wikipedia.org/wiki/Common_Gateway_Interface
	em["SERVER_SOFTWARE"] = "TODO";
	em["SERVER_NAME"] = "ALSO TODO";
	em["REQUEST_METHOD"] = m_request.getMethodAsString();
	em["PATH_INFO"] = m_request.getLocation();
	em["QUERY_STRING"] = m_request.getQueryString();

	Popen pop = my_popen(command, filename, em);

	if (pop.status != 200)
		return pop.status;

#define BUF_LEN 4096

	char buf[BUF_LEN];

	read(pop.readfd, buf, BUF_LEN - 1);
	buf[BUF_LEN - 1] = 0;
	print(buf);

	pop.closeFDs();
	return pop.status;
}
