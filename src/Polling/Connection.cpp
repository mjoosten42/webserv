#include "Connection.hpp"

#include "FD.hpp"
#include "Listener.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "buffer.hpp" // buf
#include "logger.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

#include <queue>

Connection::Connection() {}

Connection::Connection(FD m_fd, const Listener *listener, const std::string &peer):
	m_fd(m_fd), m_listener(listener), m_peer(peer), m_close(false) {}

FD Connection::receive(short &events) {
	ssize_t bytes_received = WS::read(m_fd);
	FD		source_fd;

	LOG(CYAN "Received: " DEFAULT << bytes_received);
	switch (bytes_received) {
		case -1:
			m_close = true;
		case 0: // No data although POLLIN
			break;
		default:

			// LOG(YELLOW << std::string(winSize(), '-'));
			// LOG(std::string(buf, bytes_received));
			// LOG(std::string(winSize(), '-') << DEFAULT);

			Response &response = getLastResponse();
			Request	 &request  = response.getRequest();

			request.append(buf, bytes_received);

			// LOG(request);

			if (request.getState() == STARTLINE || request.getState() == HEADERS)
				break;

			if (!response.hasProcessedRequest()) { // Do once
				response.m_peer = m_peer;
				response.addServer(&m_listener->getServerByHost(request.getHost()));
				response.processRequest();

				if (response.isCGI())
					source_fd = response.getSourceFD();
				else
					setFlag(events, POLLOUT);
			}

			if (response.isCGI() && !request.getBody().empty())
				response.writeToCGI();
	}
	return source_fd;
}

// Send data back to a client
// This should only be called if POLLOUT is set
FD Connection::send(short &events) {
	Response	&response	= m_responses.front();
	std::string &chunk		= response.getNextChunk();
	ssize_t		 bytes_sent = WS::write(m_fd, chunk);
	FD			 source_fd;

	LOG(CYAN "Send: " DEFAULT << bytes_sent);
	switch (bytes_sent) {
		case -1:
			m_close = true;
			break;
		default:

			// LOG(YELLOW << std::string(winSize(), '-'));
			// LOG(chunk.substr(0, bytes_sent));
			// LOG(std::string(winSize(), '-') << DEFAULT);

			response.trimChunk(bytes_sent);
			if (response.isDone()) {
				m_close |= response.wantsClose();
				if (response.hadFD())
					source_fd = response.getSourceFD();
				m_responses.pop();
			} else if (!response.isCGI())
				setFlag(events, POLLOUT);
	}
	return source_fd;
}

Response &Connection::getLastResponse() {
	if (m_responses.empty() || m_responses.back().getRequest().getState() == DONE)
		m_responses.push(Response());
	return m_responses.back();
}

bool Connection::wantsClose() const {
	return m_close;
}
