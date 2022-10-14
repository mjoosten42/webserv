#include "Handler.hpp"
#include "Request.hpp"
#include "utils.hpp"

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

//  TODO: envp map

bool my_exec(int infd, int outfd, const std::string& path, const std::string& filename, const char **envp) {
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

Popen my_popen(const std::string& path, const std::string& filename, const char **envp) {
	Popen popen;
	int	  serverToCgi[2];
	int	  cgiToServer[2];

	popen.status = 500;
	if (pipe(serverToCgi) == -1) {
		return popen;
	} else if (pipe(cgiToServer) == -1) {
		close(serverToCgi[0]);
		close(serverToCgi[1]);
		return popen;
	}

	popen.readfd  = cgiToServer[0];
	popen.writefd = serverToCgi[1];

	popen.pid	  = fork();
	if (popen.pid == -1) {
		close(serverToCgi[0]);
		close(serverToCgi[1]);
		close(cgiToServer[0]);
		close(cgiToServer[1]);
		return popen;
	} else if (popen.pid == 0) {
		close(serverToCgi[1]);
		close(cgiToServer[0]);
		if (my_exec(serverToCgi[0], cgiToServer[1], path, filename, envp) == false)
			exit(EXIT_FAILURE);
	} else {
		close(serverToCgi[0]);
		close(cgiToServer[1]);
	}

	popen.status = 200;
	return popen;
}

int handleCGI(const std::string& command, const std::string& filename) {
	extern char **environ;
	Popen		  pop = my_popen(command, filename, const_cast<const char **>(environ));

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
