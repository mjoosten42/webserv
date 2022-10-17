#include "Connection.hpp"

#include "Handler.hpp"
#include "Server.hpp"
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
			if (errno != ECONNRESET && errno != ETIMEDOUT)
				fatal_perror("recv");
			std::cerr << RED "INFO: Connection reset or timeout\n" DEFAULT;
		case 0:
			unsetFlag(events, POLLOUT);
			break;
		default:
			buf[bytes_received] = 0;

			std::cout << RED "----START BUF" << std::string(40, '-') << DEFAULT << std::endl;
			std::cout << buf;
			std::cout << RED "----END BUF" << std::string(40, '-') << DEFAULT << std::endl;

			m_request.add(buf);

			if (bytes_received != BUFSIZE) {
				m_request.ProcessRequest();
				std::cout << m_request << std::endl;
				m_responses.push(Response());
				Handler h(m_request, m_responses.back(), m_server);
				h.handle();
				m_request.clear();
				setFlag(events, POLLOUT);
			}
	}
}

//  Send data back to a client
//  This should only be called if POLLOUT is set
void Connection::sendToClient(short& events) {
	Response  & response   = m_responses.front();
	std::string str		   = response.getResponseAsString();
	ssize_t		bytes_send = send(m_fd, str.c_str(), str.length(), 0);

	std::cout << "Send: " << bytes_send << "\n";
	switch (bytes_send) {
		case -1:
			fatal_perror("send");
		default:
			m_responses.pop();
			unsetFlag(events, POLLOUT);
	}
}
