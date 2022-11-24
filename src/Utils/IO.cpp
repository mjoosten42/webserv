#include "IO.hpp"

#include "Listener.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "methods.hpp"
#include "utils.hpp"

#include <iostream>
#include <string>
#include <sys/poll.h>
#include <utility>
#include <vector>

std::ostream &operator<<(std::ostream &os, const pollfd &pfd) {
	return os << pfd.fd;
}

std::ostream &operator<<(std::ostream &os, const std::pair<int, std::string> &pair) {
	return os << "{ " << pair.first << ", " << pair.second << " }";
}

std::ostream &operator<<(std::ostream &os, const std::vector<methods> &methods) {
	os << "{ ";
	for (auto method : methods)
		os << toString(method) << " ";
	return os << " }";
}

std::ostream &operator<<(std::ostream &os, const Request &request) {
	os << MAGENTA "State: " DEFAULT << request.getStateAsString() << "\n";
	os << MAGENTA "Method: " DEFAULT << toString(request.getMethod()) << "\n";
	os << MAGENTA "Location: " DEFAULT << request.getLocation() << "\n";
	os << MAGENTA "Query string: " DEFAULT << request.getQueryString() << "\n";
	os << MAGENTA "Path info: " DEFAULT << request.getPathInfo() << "\n";
	os << MAGENTA "Headers: {\n" DEFAULT << request.getHeadersAsString("\t") << MAGENTA "}\n" DEFAULT;
	os << MAGENTA "Host: " DEFAULT << request.getHost() << "\n";
	os << MAGENTA "Content-Length: " DEFAULT << request.getContentLength() << "\n";
	os << MAGENTA "Body: [" DEFAULT << request.getBody() << MAGENTA "]\n" DEFAULT;
	os << MAGENTA "Body total: " DEFAULT << request.getBodyTotal() << "\n";
	os << MAGENTA "Status: " DEFAULT << request.getStatus();
	return os;
}

std::string Listener::getListenerAsString(std::string tabs) const {
	std::string ret = tabs + "Listener {\n";
	std::string end = tabs + "}";

	tabs += "\t";
	ret += tabs + "fd: " + toString(m_fd) + "\n";
	ret += tabs + "listen " + m_listenAddr + ":" + toString(m_port) + "\n";
	for (auto server : m_servers)
		ret += server.getServerAsString(tabs) + "\n";
	return ret + end;
}

std::string Server::getServerAsString(std::string tabs) const {
	std::string ret = tabs + "Server {\n";
	std::string end = tabs + "}";

	tabs += "\t";
	ret += tabs + "listen " + m_host + ":" + toString(m_port) + "\n";
	ret += tabs + "server_name " + rangeToString(m_names.begin(), m_names.end()) + "\n";
	for (auto loc : m_locations)
		ret += loc.getLocationAsString(tabs) + "\n";
	return ret + end;
}

std::string Location::getLocationAsString(std::string tabs) const {
	std::string ret = tabs + "Location " + m_location + " {\n";
	std::string end = tabs + "}";

	tabs += "\t";
	ret += tabs + "root " + m_root + "\n";
	ret += tabs + "index " + m_indexPage + "\n";
	ret += tabs + "error_pages " + rangeToString(m_error_pages.begin(), m_error_pages.end()) + "\n";
	ret += tabs + "limit_except " + rangeToString(m_limit_except.begin(), m_limit_except.end()) + "\n";
	ret += tabs + "redirect " + m_redirect + "\n";
	ret += tabs + "cgi " + rangeToString(m_CGIs.begin(), m_CGIs.end()) + "\n";
	ret += tabs + "upload " + m_uploadDir + "\n";
	ret += tabs + "client_max_body_size " + toString(m_client_max_body_size) + "\n";
	ret += tabs + "autoindex " + (m_auto_index ? "on" : "off") + "\n";
	return ret + end;
}
