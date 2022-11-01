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

	// TESTING getLocationIndexForFile
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

	// TESTING getLocationIndexForAddress
	loc_index = s.getLocationIndexForAddress("/images/amogus.jpg");
	REQUIRE( loc_index == 1);
	test = s.translateAddressToPath(loc_index, "/images/amogus.jpg");
	REQUIRE( test == "html/img/amogus.jpg"); //Check loc 1 for file it contains

	loc_index = s.getLocationIndexForAddress("/yeet/amogus_bogus_file");
	REQUIRE( loc_index == 2);
	test = s.translateAddressToPath(loc_index, "/yeet/amogus_bogus_file");
	REQUIRE( test == "html/second_dir/amogus_bogus_file"); //Check loc 2 for file it contains

	test = s.translateAddressToPath(1, "/yeet/amogus_bogus_file");
	REQUIRE( test == "");  //Check loc 1 for file in loc 2

	test = s.translateAddressToPath(2, "/images/amogus.jpg");
	REQUIRE( test == "");  //Check loc 2 for file in loc 1

	test = s.translateAddressToPath(2, "n");
	REQUIRE( test == "");  //Check loc 2 for non existent file

	loc_index = s.getLocationIndexForAddress("/image/hoi.jopg");
	REQUIRE( loc_index == 0);
	test = s.translateAddressToPath(loc_index, "/image/hoi.jopg");
	REQUIRE( test == "html/image/hoi.jopg");

}
