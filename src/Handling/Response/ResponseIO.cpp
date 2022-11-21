#include "Response.hpp"
#include "logger.hpp"
#include "syscalls.hpp"

#include <string>

std::string Response::readBlock() {
	ssize_t		bytes_read = WS::read(m_source_fd);
	std::string block;

	LOG(CYAN "Read: " DEFAULT << bytes_read);
	switch (bytes_read) {
		case -1:
		case 0:
			m_doneReading = true;
			break;
		default:
			// LOG(YELLOW << std::string(winSize(), '-'));
			// LOG(std::string(buf, bytes_read));
			// LOG(std::string(winSize(), '-') << DEFAULT);

			block.append(buf, bytes_read);
	}
	return block;
}

void Response::writeToCGI() {
	std::string &body		   = m_request.getBody();
	int			 fd			   = m_cgi.popen.writefd;
	ssize_t		 bytes_written = WS::write(fd, body);

	LOG(CYAN "Write: " DEFAULT << bytes_written);
	switch (bytes_written) {
		case -1:
			body.clear();
			break;
		default:
			// LOG(YELLOW << std::string(winSize(), '-'));
			// LOG(body.substr(0, bytes_written));
			// LOG(std::string(winSize(), '-') << DEFAULT);

			body.erase(0, bytes_written);
	}
}
