#include "catch_amalgamated.hpp"

#include "Listener.hpp"
#include "logger.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "file.hpp"

TEST_CASE( "Hosts", "[Server]" ) {
	std::string ext;
	
	ext = "Some test";
	ext = extension(ext);
	REQUIRE( ext == "");
	ext = "Test.py";
	ext = extension(ext);
	REQUIRE( ext == "py");
	ext = "Test..py";
	ext = extension(ext);
	REQUIRE( ext == "py");
	ext = "Test.";
	ext = extension(ext);
	REQUIRE( ext == "");
}
