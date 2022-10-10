#include "Poller.hpp"

#include "GetStaticFileTransfer.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <sys/socket.h> // accept
#include <unistd.h>		// close

#define BUFSIZE 2048

Poller::Poller(const Server *servers, int size): m_servers(servers), m_nb_servers(size) {
	for (const Server *it = m_servers; it != m_servers + size; it++) {
		pollfd client = { it->getFD(), POLLIN, 0 };
		m_pollfds.push_back(client);
		m_fdToServerMap[it->getFD()] = it;
	}
}

Poller::~Poller() {}

//  accepts a client
void Poller::acceptClient(int serverfd) {
	int			  fd	 = accept(serverfd, NULL, NULL);
	pollfd		  client = { fd, POLLIN, 0 };
	const Server *server = m_fdToServerMap[serverfd];

	if (fd < 0)
		fatal_perror("accept");

	set_fd_nonblocking(fd);

	m_pollfds.push_back(client);
	m_requestResponse[fd] = stuff(fd, server);

	std::cout << "NEW CLIENT: " << fd << std::endl;
}

//  when POLLIN is set, receives a message from the client
//  returns true on success, false when the client sents EOF.
bool Poller::receiveFromClient(int fd) {
	static int num_recvs = 0;
	char	   buf[BUFSIZE];
	ssize_t	   recv_len = recv(fd, buf, BUFSIZE - 1, 0);

	buf[recv_len]		= 0;

	if (recv_len == -1) {
		fatal_perror("recv");
	} else if (recv_len == 0) {
		return false;
	}

	m_requestResponse[fd].request.add(buf);
	m_requestResponse[fd].request.stringToData();

	num_recvs++;

	//  TODO; probably some stuff should be delegated to some other class here
	//  Response response;
	//  response.m_fd			= fd;

	//  std::string bodyContent = "server FD: " + std::to_string(m_fdservermap[fd]->getFD());
	//  bodyContent += ", num receives: " + std::to_string(num_recvs) + "\n";

	//  response.m_statusLine = "HTTP/1.1 200 OK";

	//  response.m_header.push_back("Cache-Control: no-cache");
	//  response.m_header.push_back("Server: amogus");
	//  response.m_header.push_back("Date: Wed Jul 4 15:32:03 2012");
	//  response.m_header.push_back("Connection: Keep-Alive:");
	//  response.m_header.push_back("Content-Type: text/plain");
	//  response.m_header.push_back("Content-Length: " + std::to_string(bodyContent.length()));

	//  response.m_body.push_back(bodyContent);

	//  std::string number_str = std::to_string(num_recvs); // WAS UNUSED.

	//  std::string toSend = "server FD: " + std::to_string(m_fdservermap[fd]->getFD());
	//  toSend += ", num receives: " + std::to_string(num_recvs) + "\n";

	//  std::string number_str = std::to_string(num_recvs);
	//  std::string str("HTTP/1.1 200 OK\r\n"
	//  				"Server: amogus\r\n"
	//  				"Date: Wed Jul 4 15:32:03 2012\r\n"
	//  				"Connection: Keep-Alive:\r\n"
	//  				"Content-Type: text/plain\r\n"
	//  				"Content-Length: ");
	//  str += std::to_string(toSend.length());
	//  str += "\r\n\r\n";
	//  str += toSend;
	//  str += "\r\n\r\n";

	//  if (send(fd, str.c_str(), str.length(), 0) == -1)
	//  	fatal_perror("send");

	handleGetWithStaticFile(fd, m_requestResponse[fd].request.getLocation());
	//  handleGetWithStaticFile(fd, "Notes");
	//   response.sendResponse();
	return true;
}

void Poller::start() {
	while (true) {
		std::cout << "\n----STARTING LOOP----\n";

		printPollFds(m_pollfds);
	
		int poll_status = poll(m_pollfds.data(), static_cast<nfds_t>(m_pollfds.size()), -1);

		if (poll_status == -1)
			fatal_perror("poll");

		else if (poll_status == 0)
			std::cerr << "No events? wtf??\n";

		//  Loop over servers for new clients
		for (size_t i = 0; i < m_nb_servers; i++)
			if (m_pollfds[i].revents & POLLIN)
				acceptClient(m_pollfds[i].fd);

		//  loop over clients for new messages
		for (size_t i = m_nb_servers; i < m_pollfds.size(); i++) {
			if (m_pollfds[i].revents & (POLLERR | POLLNVAL))
				std::cerr << "Client " << m_pollfds[i].fd << " error\n";
			if (m_pollfds[i].revents & POLLHUP)
				removeClient(m_pollfds[i--].fd);
			else if (m_pollfds[i].revents & POLLIN)
				receiveFromClient(m_pollfds[i].fd);
		}
	}
}

void Poller::removeClient(int fd) {
	m_fdToServerMap.erase(fd);

	for (size_t i = 0; i < m_pollfds.size(); i++)
		if (m_pollfds[i].fd == fd)
			m_pollfds.erase(m_pollfds.begin() + i);

	m_requestResponse.erase(fd);

	if (close(fd) < 0) {
		std::cerr << "Fd " << fd << ": ";
		fatal_perror("close");
	}

	std::cout << "CLIENT " << fd << " LEFT\n";
}
