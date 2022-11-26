#include "syscalls.hpp"

#include "EnvironmentMap.hpp"
#include "buffer.hpp" // buf
#include "logger.hpp"
#include "utils.hpp" // set_fd_nonblocking

#include <fcntl.h>	// open
#include <stdlib.h> // realpath
#include <string.h> // strerror
#include <string>
#include <sys/socket.h> // recv, send, accept
#include <sys/wait.h>	// wait
#include <unistd.h>		// close, write, fork, chdir, execve

namespace WS {

int close(int fd) {
	int ret = ::close(fd);

	if (ret == -1)
		perror("close");
	return ret;
}

ssize_t read(int fd) {
	ssize_t ret = ::read(fd, buf, BUFFER_SIZE);

	if (ret == -1)
		LOG_ERR("read(" << fd << "): " << strerror(errno));
	return ret;
}

ssize_t write(int fd, const std::string &str) {
	ssize_t ret = ::write(fd, str.data(), str.length());

	if (ret == -1)
		if (errno != EPIPE)
			LOG_ERR("write(" << fd << "): " << strerror(errno));
	return ret;
}

pid_t wait() {
	int	  status;
	pid_t ret = ::wait(&status);

	if (ret == -1)
		LOG_ERR("wait: " << strerror(errno));
	return ret;
}

int chdir(const std::string &path) {
	int ret = ::chdir(path.c_str());

	if (ret == -1)
		LOG_ERR("chdir(\"" << path << "\"): " << strerror(errno));
	return ret;
}

int execve(const std::string &path, char *const argv[], const EnvironmentMap &em) {
	int ret = ::execve(path.c_str(), argv, em.toCharpp());

	if (ret == -1)
		LOG_ERR("execve(\"" << path << "\"): " << strerror(errno));
	return ret; // always -1
}

int poll(std::vector<pollfd> &pollfds) {
	int ret = ::poll(pollfds.data(), static_cast<nfds_t>(pollfds.size()), -1);

	if (ret == -1 && errno != EINTR) // SIGCHLD causes EINTR
		LOG_ERR("poll: " << strerror(errno));
	if (ret == 0)
		LOG_ERR("poll returned zero");
	return ret;
}

int accept(int fd, sockaddr *peer) {
	socklen_t size = sizeof(sockaddr);
	int		  ret  = ::accept(fd, peer, &size);

	if (ret == -1)
		LOG_ERR("accept(" << fd << "): " << strerror(errno));
	return ret;
}

std::string realpath(const std::string &relative_path) {
	const char *ret = ::realpath(relative_path.c_str(), path);

	if (!ret) {
		LOG_ERR("realpath(\"" << relative_path << "\"): " << strerror(errno));
		throw 404;
	}
	return ret;
}

} // namespace WS
