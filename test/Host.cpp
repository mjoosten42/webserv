#include "catch_amalgamated.hpp"

#include "Listener.hpp"
#include "logger.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"

TEST_CASE( "Hosts", "[Server]" ) {
	ConfigParser config;
	config.parse_config("default.conf");

	std::vector<Listener> listeners;
	initFromConfig(config, listeners);

	std::string test = listeners.front().getServerByHost("amogus.localhst.co.uk").getRootForFile("amogus.jpg");

	REQUIRE( test == "html/img");
}
