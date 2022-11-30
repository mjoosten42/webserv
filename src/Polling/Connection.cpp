#include "Connection.hpp"

#include "FD.hpp"
#include "Listener.hpp"
#include "Poller.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "buffer.hpp" // buf
#include "logger.hpp"
#include "utils.hpp"

#include <queue>
#include <sys/socket.h> // recv, send

Connection::Connection() {}

Connection::Connection(FD fd, const Listener *listener, Poller *poller, const std::string &peer):
	m_fd(fd), m_listener(listener), m_poller(poller), m_peer(peer), m_close(false) {
	(void)m_poller;
}

short Connection::receive() {
	ssize_t bytes_received = ::recv(m_fd, buf, BUFFER_SIZE, 0);
	short	flag		   = 0;

	LOG(CYAN "Received: " DEFAULT << bytes_received);
	switch (bytes_received) {
		case -1:
			perror("recv");
		case 0:
			m_close = true;
			break;
		default:

			// LOG(YELLOW << std::string(winSize(), '-'));
			// LOG(std::string(buf, bytes_received));
			// LOG(std::string(winSize(), '-') << DEFAULT);

			Response &response = getLastResponse();
			Request	 &request  = response.getRequest();

			request.append(buf, bytes_received);

			// LOG(request);

			if (request.getState() == STARTLINE || request.getState() == HEADERS) {
				flag |= POLLIN;
				break;
			}

			if (!response.hasProcessedRequest()) { // Do once
				response.m_peer = m_peer;
				response.addServer(&m_listener->getServerByHost(request.getHost()));
				response.processRequest();

				if (!response.isCGI())
					flag |= POLLOUT;
			}

			if (request.getBody().size() < BUFFER_SIZE)
				flag |= POLLIN;
	}
	LOG(CYAN "Returning " DEFAULT << getEventsAsString(flag));
	return flag;
}

// Send data back to a client
// This should only be called if POLLOUT is set
short Connection::send() {
	Response	&response	= m_responses.front();
	std::string &chunk		= response.getNextChunk();
	ssize_t		 bytes_sent = ::send(m_fd, chunk.data(), chunk.size(), 0);
	short		 flag		= 0;

	LOG(CYAN "Send: " DEFAULT << bytes_sent);
	switch (bytes_sent) {
		case -1:
			perror("send");
		case 0:
			m_close = true;
			break ;
		default:

			// LOG(YELLOW << std::string(winSize(), '-'));
			// LOG(chunk.substr(0, bytes_sent));
			// LOG(std::string(winSize(), '-') << DEFAULT);

			response.trimChunk(bytes_sent);
			if (response.isDone()) {
				m_close |= response.wantsClose();
				m_responses.pop();
			}

			if (m_responses.empty())
				flag |= POLLIN;
			else
				flag |= POLLOUT;
	}
	LOG(CYAN "Returning " DEFAULT << getEventsAsString(flag));
	return flag;
}

Response &Connection::getLastResponse() {
	if (m_responses.empty() || m_responses.back().getRequest().getState() == DONE)
		m_responses.push(Response());
	return m_responses.back();
}

bool Connection::wantsClose() const {
	return m_close;
}
