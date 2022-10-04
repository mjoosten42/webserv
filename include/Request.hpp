#pragma once

#include <string>
#include <vector>

using std::string;

class Request {
	public:
		Request(const string& total);

	private:
		std::vector<std::pair<string, string> > headers;
		string									body;
		int										fd;
};
