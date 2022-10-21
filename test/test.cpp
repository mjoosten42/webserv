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

TEST_CASE( "string endswith", "[string]" ) {
	REQUIRE( strEndsWith("ab", "ab") == true );
	REQUIRE( strEndsWith("ab", "ac") == false );
	REQUIRE( strEndsWith("ab", "") == true );
	REQUIRE( strEndsWith("fab", "ab") == true );
	REQUIRE( strEndsWith("ab", "b") == true );
	REQUIRE( strEndsWith("b", "b") == true );
	REQUIRE( strEndsWith("b", "ab") == false );
	REQUIRE( strEndsWith("ba", "ba") == true );
	REQUIRE( strEndsWith("", "ab") == false );
	REQUIRE( strEndsWith("", "") == true );
}
