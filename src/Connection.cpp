#include "Connection.hpp"

#include "Listener.hpp"
#include "buffer.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <queue>
#include <sys/socket.h> //recv, send
#include <unistd.h>		// close

Connection::Connection() {}

Connection::Connection(int m_fd, const Listener *listener): m_fd(m_fd), m_listener(listener), m_close(false) {}

int Connection::receiveFromClient(short& events) {
	ssize_t bytes_received = recv(m_fd, buf, BUFFER_SIZE, 0);

	LOG(RED "Received: " DEFAULT << bytes_received);
	switch (bytes_received) {
		case -1:
			perror("recv");
		case 0:
			break;
		default:
			LOG(RED << std::string(winSize(), '-') << DEFAULT);
			LOG(std::string(buf, bytes_received));
			LOG(RED << std::string(winSize(), '-') << DEFAULT);

			Response& response = getLastResponse();
			Request & request  = response.getRequest();

			try {
				request.append(buf, bytes_received);
			} catch (int error) {
				response.m_statusCode = error;
			}

			LOG(request);
			if (request.getState() == BODY || request.getState() == DONE) {
				if (!response.hasProcessedRequest()) { // Do once
					response.addServer(&(m_listener->getServerByHost(request.getHost())));
					response.processRequest();

					if (response.needsSourceFd())
						return response.getSourceFD();
					else
						setFlag(events, POLLOUT);
				} else
					response.appendBodyPiece(request.getBody());
			}
	}
	return -1;
}

// Send data back to a client
// This should only be called if POLLOUT is set
// TODO: when send() doesn't send the entire chunk and response.isDone(), the function fails.
// when fixing, it should bear in mind that the readfd should still be removed and POLLOUT is set!
int Connection::sendToClient(short& events) {
	Response   & response	= m_responses.front();
	std::string& str		= response.getNextChunk();
	ssize_t		 bytes_sent = send(m_fd, str.data(), str.length(), 0);

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
				m_close = response.wantsClose();
				m_responses.pop();
				return response.getSourceFD();
			}
			if (response.getSourceFD() == -1)
				setFlag(events, POLLOUT);
	}
	return -1;
}

Response& Connection::getLastResponse() {
	if (m_responses.empty() || m_responses.back().getRequest().getState() == DONE)
		m_responses.push(Response());
	return m_responses.back();
}

bool Connection::wantsClose() const {
	return m_close;
}
