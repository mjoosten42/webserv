#include "catch_amalgamated.hpp"

#include "Listener.hpp"
#include "logger.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"

// TODO: fix this test
TEST_CASE( "Hosts", "[Server]" ) {
	ConfigParser config;
	config.parse_config("default.conf");

	std::vector<Listener> listeners;
	initFromConfig(config, listeners);

	std::string test = listeners.front().getServerByHost("amogus.localhst.co.uk").getRootForFile(0, "amogus.jpg");

	REQUIRE( test == "html/img");
}
