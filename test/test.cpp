#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include "stringutils.hpp"
#include "utils.hpp"


TEST_CASE( "string uppercase", "[string]" ) {
	std::string derp = "derp123";
	strToUpper(derp);
	REQUIRE( derp == std::string("DERP123") );
}

TEST_CASE( "string lowercase", "[string]" ) {
	std::string derp = "DERP123";
	strToLower(derp);
	REQUIRE( derp == std::string("derp123") );
}

TEST_CASE( "isDirectory", "[isDirectory] " ) {
	REQUIRE( isDirectory("test") == true );
	REQUIRE( isDirectory("src") == true );
	REQUIRE( isDirectory("asfdsafasdf234rasdf") == false );
	REQUIRE( isDirectory("Makefile") == false );
	REQUIRE( isDirectory("src/main.cpp") == false );
	REQUIRE( isDirectory("") == false );
}
