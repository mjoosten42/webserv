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

			if (request.getState() >= BODY) { // TODO: == BODY
				std::cout << request << std::endl;
				response.addServer(m_listener->getServerByHost(request.getHost()));
				response.processRequest();
				setFlag(events, POLLOUT); // TODO
			}
	}
}

//  Send data back to a client
//  This should only be called if POLLOUT is set
void Connection::sendToClient(short& events) {
	Response  & response   = m_responses.front();
	std::string str		   = response.getNextChunk();
	ssize_t		bytes_sent = send(m_fd, str.c_str(), str.length(), 0);

	std::cout << "Send: " << bytes_sent << "\n";
	switch (bytes_sent) {
		case -1:
			fatal_perror("send"); // TODO
		default:
			response.trimChunk(bytes_sent);
			if (!response.isDone())
				setFlag(events, POLLOUT);
			else
				m_responses.pop();
	}
}

Response& Connection::getLastResponse() {
	if (m_responses.empty() || m_responses.back().getRequest().getState() == DONE)
		m_responses.push(Response());
	return m_responses.back();
}
