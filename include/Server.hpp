#pragma once

class Server {
	public:
		Server();
		Server(int port);
		~Server();

		int getFD() const;

	private:
		int m_fd;
};
