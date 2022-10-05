#pragma once

class Server {
	public:
		Server();
		~Server();

		int getFD() const;

		void setup(int port);

	private:
		int m_fd;
};
