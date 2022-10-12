#include "Connection.hpp"

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

//  Tell a connection that it can receive data.
//  Read it, and add it to the first request (FIFO)
//  Once a request has been processed, create a response and pop the request
void Connection::receiveFromClient() {
	static char buf[BUFSIZE + 1];
	ssize_t		recv_len = recv(m_fd, buf, BUFSIZE, 0);

	switch (recv_len) {
		case -1:
			if (errno != ECONNRESET && errno != ETIMEDOUT)
				fatal_perror("recv");
			std::cerr << RED "INFO: Connection reset or timeout\n" DEFAULT;
		case 0:
			return;
		default:
			buf[recv_len] = 0;

			std::cout << RED "----START BUF" << std::string(80, '-') << DEFAULT << std::endl;
			std::cout << buf << std::endl;
			std::cout << RED "----END BUF" << std::string(80, '-') << DEFAULT << std::endl;

			//  Add a new request if none exist;
			if (m_requests.empty())
				m_requests.push(Request());

			Request& request = m_requests.front();
			request.add(buf);
			if (recv_len != BUFSIZE)
				request.ProcessRequest();

			//  TMP
			m_responses.push(Response());
			m_responses.front().m_statusCode = 200;
	}
}

void Connection::sendToClient() {
	Response  & response   = m_responses.front();
	std::string str		   = response.getResponseAsString();
	ssize_t		bytes_send = send(m_fd, str.c_str(), str.length(), 0);

	switch (bytes_send) {
		case -1:
			fatal_perror("send");
		default:
			std::cout << str.length() << std::endl;
			if (str.length() != BUFSIZE)
				m_responses.pop();
	}
}
