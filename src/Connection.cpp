#include "Connection.hpp"

#include "Listener.hpp"
#include "buffer.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <queue>
#include <sys/socket.h> //recv, send
#include <unistd.h>		// close

Connection::Connection(): m_fd(-1), m_listener(NULL) {}

Connection::Connection(int m_fd, const Listener *listener): m_fd(m_fd), m_listener(listener) {}

int Connection::receiveFromClient(short& events) {
	ssize_t bytes_received = recv(m_fd, buf, BUFFER_SIZE, 0);

	LOG(RED "Received: " DEFAULT << bytes_received);
	switch (bytes_received) {
		case -1:
			perror("recv");
		case 0:
			break;
		default:
			// LOG(RED << std::string(winSize(), '-') << DEFAULT);
			// LOG(std::string(buf, bytes_received));
			// LOG(RED << std::string(winSize(), '-') << DEFAULT);

			Response& response = getLastResponse();
			Request & request  = response.getRequest();

			request.append(buf, bytes_received);

			if (request.getState() >= BODY) { // TODO: == BODY
				if (!response.hasProcessedRequest()) {
					LOG(request);
					response.addServer(&(m_listener->getServerByHost(request.getHost())));
					response.processRequest();

					int readfd = response.getReadFD();
					if (readfd == -1)
						setFlag(events, POLLOUT);
					return readfd;
				}
			}
	}
	return -1;
}

// Send data back to a client
// This should only be called if POLLOUT is set
// TODO: when send() doesn't send the entire chunk and response.isDone(), the function fails.
// when fixing, it should bear in mind that the readfd should still be removed and POLLOUT is set!
std::pair<bool, int> Connection::sendToClient(short& events) {
	Response   & response	 = m_responses.front();
	std::string& str		 = response.getNextChunk();
	ssize_t		 bytes_sent	 = send(m_fd, str.data(), str.length(), 0);
	bool		 shouldClose = false;

	LOG(RED "Send: " DEFAULT << bytes_sent);

	// LOG(RED << std::string(winSize(), '-') << DEFAULT);
	// LOG(str.substr(0, bytes_sent));
	// LOG(RED << std::string(winSize(), '-') << DEFAULT);

	switch (bytes_sent) {
		case -1:
			fatal_perror("send"); // TODO
		default:
			response.trimChunk(bytes_sent);
			if (response.isDone()) {
				shouldClose = response.shouldClose();
				m_responses.pop();
				return std::make_pair(shouldClose, response.getReadFD());
			}
			if (response.getReadFD() == -1)
				setFlag(events, POLLOUT);
	}
	return std::make_pair(shouldClose, -1);
}

Response& Connection::getLastResponse() {
	if (m_responses.empty() || m_responses.back().getRequest().getState() == DONE)
		m_responses.push(Response());
	return m_responses.back();
}
