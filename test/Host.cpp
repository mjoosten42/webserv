#include "catch_amalgamated.hpp"

#include "Listener.hpp"
#include "logger.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"

// TODO: fix this test
TEST_CASE( "Hosts", "[Server]" ) {
	ConfigParser config;
	config.parse_config("test.conf");

	std::vector<Listener> listeners;
	initFromConfig(config, listeners);

	Server s = listeners.front().getServerByHost("amogus.localhst.co.uk");
	int loc_index;
	std::string test;

	loc_index = s.getLocationIndexForFile("amogus.jpg");
	REQUIRE( loc_index == 1);
	test = s.getRootForFile(loc_index, "amogus.jpg");
	REQUIRE( test == "html/img"); //Check loc 1 for file it contains

	loc_index = s.getLocationIndexForFile("amogus_bogus_file");
	REQUIRE( loc_index == 2);
	test = s.getRootForFile(loc_index, "amogus_bogus_file");
	REQUIRE( test == "html/second_dir"); //Check loc 2 for file it contains

	test = s.getRootForFile(1, "amogus_bogus_file");
	REQUIRE( test == "");  //Check loc 1 for file in loc 2

	test = s.getRootForFile(-1, "amogus.jpg");
	REQUIRE( test == "html/img"); //Check whole server for file in loc 0

	loc_index = s.getLocationIndexForFile("404.html");
	REQUIRE( loc_index == 0);
	test = s.getRootForFile(loc_index, "404.html");
	REQUIRE( test == "html"); // Check for file in server root
}
