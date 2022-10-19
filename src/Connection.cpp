#include "Connection.hpp"

#include "Listener.hpp"
#include "defines.hpp"
#include "utils.hpp"

#include <queue>
#include <sys/socket.h> //recv, send
#include <unistd.h>		// close

#define RECV_BUF_SIZE 2048

Connection::Connection(): m_fd(-1), m_listener(NULL) {}

Connection::Connection(int m_fd, const Listener *listener): m_fd(m_fd), m_listener(listener) {}

//  Read new data, and add it to the first request (FIFO)
//  Once a request has been processed, create a response and pop the request
void Connection::receiveFromClient(short& events) {
	static char buf[RECV_BUF_SIZE + 1];
	ssize_t		bytes_received = recv(m_fd, buf, RECV_BUF_SIZE, 0);

	std::cout << "Received: " << bytes_received << "\n";
	switch (bytes_received) {
		case -1:
			fatal_perror("recv");
		case 0:
			break;
		default:
			buf[bytes_received] = 0;

			std::cout << RED "----START BUF" << std::string(40, '-') << DEFAULT << std::endl;
			std::cout << buf << std::endl;
			std::cout << RED "----END BUF" << std::string(40, '-') << DEFAULT << std::endl;

			Response& response = getLastResponse();
			Request & request  = response.getRequest();

			request.add(buf);

			if (request.getState() == BODY) {
				std::cout << request << std::endl;
				response.addServer(m_listener->getServerByHost(request.getHost()));
				response.processNextChunk();
				setFlag(events, POLLOUT); // TODO
			}
	}
}

//  Send data back to a client
//  This should only be called if POLLOUT is set
void Connection::sendToClient(short& events) {
	Response  & response   = m_responses.front();
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
			setFlag(events, POLLOUT);
		} else {
			m_responses.pop();
		}
	}
}

Response& Connection::getLastResponse() {
	if (m_responses.empty())
		m_responses.push(Response());
	return m_responses.back();
}
