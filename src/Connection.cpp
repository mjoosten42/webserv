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
	int		source_fd	   = -1;

	LOG(RED "Received: " DEFAULT << bytes_received);
	switch (bytes_received) {
		case -1:
			perror("recv");
		case 0:
			m_close = true;
			break;
		default:

			// LOG(RED << std::string(winSize(), '-') << DEFAULT);
			// LOG(std::string(buf, bytes_received));
			// LOG(RED << std::string(winSize(), '-') << DEFAULT);

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

					if (response.hasSourceFd())
						source_fd = response.getSourceFD();
					else
						setFlag(events, POLLOUT);
				}
				response.appendBodyPiece();
			}
	}
	return source_fd;
}

// Send data back to a client
// This should only be called if POLLOUT is set
int Connection::sendToClient(short& events) {
	Response   & response	= m_responses.front();
	std::string& chunk		= response.getNextChunk();
	ssize_t		 bytes_sent = send(m_fd, chunk.data(), chunk.length(), 0);
	int			 source_fd	= -1;

	LOG(RED "Send: " DEFAULT << bytes_sent);

	switch (bytes_sent) {
		case -1:
			perror("send");
			m_close = true;
			break;
		default:

			// LOG(RED << std::string(winSize(), '-') << DEFAULT);
			// LOG(str.substr(0, bytes_sent));
			// LOG(RED << std::string(winSize(), '-') << DEFAULT);

			response.trimChunk(bytes_sent);
			if (response.isDone()) {
				m_close |= response.wantsClose();
				source_fd = response.getSourceFD();
				m_responses.pop();
			} else if (!response.hasSourceFd())
				setFlag(events, POLLOUT);
	}

	return source_fd;
}

Response& Connection::getLastResponse() {
	if (m_responses.empty() || m_responses.back().getRequest().getState() == DONE)
		m_responses.push(Response());
	return m_responses.back();
}

bool Connection::wantsClose() const {
	return m_close;
}
