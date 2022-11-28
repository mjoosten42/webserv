#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include "stringutils.hpp"


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

TEST_CASE( "toHex", "[string]" ) {
	int derp = 2147483647;

	REQUIRE( toHex(derp) == "7fffffff" );
}
