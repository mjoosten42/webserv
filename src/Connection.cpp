#include "Connection.hpp"

#include "Listener.hpp"
#include "buffer.hpp"
#include "defines.hpp"
#include "utils.hpp"

#include <queue>
#include <sys/socket.h> //recv, send
#include <unistd.h>		// close

Connection::Connection(): m_fd(-1), m_listener(NULL) {}

Connection::Connection(int m_fd, const Listener *listener): m_fd(m_fd), m_listener(listener) {}

void Connection::receiveFromClient(short& events) {
	ssize_t bytes_received = recv(m_fd, buf, BUFFER_SIZE, 0);

	LOG(RED "Received: " DEFAULT << bytes_received);
	switch (bytes_received) {
		case -1:
			fatal_perror("recv");
		case 0:
			break;
		default:
			LOG(RED "----START BUF" << std::string(40, '-') << DEFAULT);
			LOG(std::string(buf, bytes_received));
			LOG(RED "----END BUF" << std::string(40, '-') << DEFAULT);

			Response& response = getLastResponse();
			Request & request  = response.getRequest();

			request.append(buf, bytes_received);

			if (request.getState() >= BODY) { // TODO: == BODY
				LOG(request);
				response.addServer(&(m_listener->getServerByHost(request.getHost())));
				response.processRequest();
				setFlag(events, POLLOUT); // TODO
			}
	}
}

//  Send data back to a client
//  This should only be called if POLLOUT is set
bool Connection::sendToClient(short& events) {
	Response   & response	 = m_responses.front();
	std::string& str		 = response.getNextChunk();
	ssize_t		 bytes_sent	 = send(m_fd, str.data(), str.length(), 0);
	bool		 shouldClose = false;

	LOG(RED "Send: " DEFAULT << bytes_sent);
	switch (bytes_sent) {
		case -1:
			fatal_perror("send"); // TODO
		default:
			response.trimChunk(bytes_sent);
			if (!response.isDone()) {
				if (!response.readfdNeedsPoll())
					setFlag(events, POLLOUT);
			} else {
				shouldClose = response.shouldClose();
				m_responses.pop();
			}
	}
	return shouldClose;
}

Response& Connection::getLastResponse() {
	if (m_responses.empty() || m_responses.back().getRequest().getState() == DONE)
		m_responses.push(Response());
	return m_responses.back();
}
