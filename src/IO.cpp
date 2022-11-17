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

std::ostream& operator<<(std::ostream& os, const pollfd& pfd) {
	return os << pfd.fd;
}

std::ostream& operator<<(std::ostream& os, const std::pair<int, std::string>& pair) {
	return os << "{ " << pair.first << ", " << pair.second << " }";
}

std::ostream& operator<<(std::ostream& os, const std::pair<methods, bool>& limit_except) {
	return os << toString(limit_except.first);
}

std::ostream& operator<<(std::ostream& os, const std::vector<methods>& methods) {
	os << "{ ";
	for (auto method : methods)
		os << toString(method) << " ";
	return os << " }";
}

std::ostream& operator<<(std::ostream& os, const Request& request) {
	os << MAGENTA "State: " DEFAULT << request.getStateAsString() << std::endl;
	os << MAGENTA "Method: " DEFAULT << toString(request.getMethod()) << std::endl;
	os << MAGENTA "Location: " DEFAULT << request.getLocation() << std::endl;
	os << MAGENTA "Query string: " DEFAULT << request.getQueryString() << std::endl;
	os << MAGENTA "Path info: " DEFAULT << request.getPathInfo() << std::endl;
	os << MAGENTA "Headers: {\n" DEFAULT << request.getHeadersAsString() << MAGENTA << "}\n";
	os << MAGENTA "Host: " DEFAULT << request.getHost() << std::endl;
	os << MAGENTA "Content-Length: " DEFAULT << request.getContentLength() << std::endl;
	os << MAGENTA "Body: " DEFAULT << request.getBody() << std::endl;
	os << MAGENTA "Body total: " DEFAULT << request.getBodyTotal() << std::endl;
	os << MAGENTA "Status: " DEFAULT << request.getStatus();
	return os;
}

std::string Listener::getListenerAsString(std::string tabs) const {
	std::string ret = tabs + "Listener {\n";

	ret += tabs + "\tfd: " + toString(m_fd) + "\n";
	ret += tabs + "\tlisten " + m_listenAddr + ":" + toString(m_port) + "\n";
	for (auto server : m_servers)
		ret += server.getServerAsString(tabs + "\t") + "\n";
	return ret + tabs + "}";
}

std::string Server::getServerAsString(std::string tabs) const {
	std::string ret = tabs + "Server {\n";

	ret += tabs + "\tlisten " + m_host + ":" + toString(m_port) + "\n";
	ret += tabs + "\tserver_name " + rangeToString(m_names.begin(), m_names.end()) + "\n";
	for (auto loc : m_locations)
		ret += loc.getLocationAsString(tabs + "\t") + "\n";
	return ret + tabs + "}";
}

std::string Location::getLocationAsString(std::string tabs) const {
	std::string ret = tabs + "Location " + m_location + " {\n";

	ret += tabs + "\troot " + m_root + "\n";
	ret += tabs + "\tindex " + m_indexPage + "\n";
	ret += tabs + "\terror_pages " + rangeToString(m_error_pages.begin(), m_error_pages.end()) + "\n";
	ret += tabs + "\tlimit_except " + rangeToString(m_limit_except.begin(), m_limit_except.end()) + "\n";
	ret += tabs + "\tredirect " + m_redirect + "\n";
	ret += tabs + "\tcgi " + rangeToString(m_CGIs.begin(), m_CGIs.end()) + "\n";
	ret += tabs + "\tclient_max_body_size " + toString(m_client_max_body_size) + "\n";
	ret += tabs + "\tautoindex " + (m_auto_index ? "on" : "off") + "\n";
	return ret + tabs + "}";
}
