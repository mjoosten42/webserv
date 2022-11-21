#include "catch_amalgamated.hpp"

#include "Listener.hpp"
#include "logger.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"
#include "utils.hpp"

TEST_CASE( "Hosts", "[Server]" ) {
	std::string ext;
	
	ext = "Some test";
	ext = getExtension(ext);
	REQUIRE( ext == "");
	ext = "Test.py";
	ext = getExtension(ext);
	REQUIRE( ext == "py");
	ext = "Test..py";
	ext = getExtension(ext);
	REQUIRE( ext == "py");
	ext = "Test.";
	ext = getExtension(ext);
	REQUIRE( ext == "");
}
