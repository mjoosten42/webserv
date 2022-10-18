#include "Connection.hpp"

#include "defines.hpp"
#include "utils.hpp"

#include <queue>
#include <sys/socket.h> //recv, send
#include <unistd.h>		// close

Connection::Connection(): m_fd(-1), m_server(NULL) {}

Connection::Connection(int m_fd, const Server *server): m_fd(m_fd), m_server(server) {
	(void)m_server;
}

//  Read new data, and add it to the first request (FIFO)
//  Once a request has been processed, create a response and pop the request
void Connection::receiveFromClient(short& events) {
	static char buf[BUFSIZE + 1];
	ssize_t		bytes_received = recv(m_fd, buf, BUFSIZE, 0);

	std::cout << "Received: " << bytes_received << "\n";
	switch (bytes_received) {
		case -1:
			fatal_perror("recv");
		case 0:
			unsetFlag(events, POLLOUT);
			break;
		default:
			buf[bytes_received] = 0;

			std::cout << RED "----START BUF" << std::string(40, '-') << DEFAULT << std::endl;
			std::cout << buf << std::endl;
			std::cout << RED "----END BUF" << std::string(40, '-') << DEFAULT << std::endl;

			m_request.add(buf);

			if (m_request.getState() == DONE) {
				m_responses.push(Response(m_request, m_server));
				m_request.clear();
				setFlag(events, POLLOUT);
			}
	}
}

//  Send data back to a client
//  This should only be called if POLLOUT is set
void Connection::sendToClient(short& events) {

	Response& response = m_responses.front();

	if (!response.isInitialized())
		response.processNextChunk();
	std::string str		   = response.getChunk();
	ssize_t		bytes_sent = send(m_fd, str.c_str(), str.length(), 0);

	std::cout << "Send: " << bytes_sent << "\n";
	if (bytes_sent == -1) {
		fatal_perror("send");
	} else if (bytes_sent < static_cast<ssize_t>(str.length())) {
		//  during testing this line has never been reached. But still, just to be sure.
		response.trimChunk(bytes_sent);
	} else {
		if (!response.isDone()) {
			response.processNextChunk();
		} else {
			m_responses.pop();
			if (m_responses.size() == 0)
				unsetFlag(events, POLLOUT);
		}
	}
}
