#pragma once

#include "shared_fd.hpp"

#include <string>

class Server {
	public:
		Server();
		Server(int port);
		~Server();

		int getFD() const;

	private:
		shared_fd	m_fd;
		std::string m_root;
};
