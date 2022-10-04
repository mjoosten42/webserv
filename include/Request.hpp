#pragma once

#include <map>
#include <string>

using std::string;

enum methods { GET, POST, DELETE };

class Request {
	public:
		Request(int fd, const string& total);

	private:
		int						 m_fd;
		methods					 m_method;
		std::map<string, string> m_headers;
		string					 m_body;
};
