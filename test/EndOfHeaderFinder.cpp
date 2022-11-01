#include "catch_amalgamated.hpp"

#include "Response.hpp"

/*
TEST_CASE( "EndOfHeaderFinder generic", "[EndOfHeaderFinder]") {

	Response rsp;

	REQUIRE( finder.find("derp") == std::string::npos );
	REQUIRE( finder.find("\n") == std::string::npos );
	REQUIRE( finder.find("\n") == 1 );

	REQUIRE( finder.find("\r\n") == std::string::npos );
	REQUIRE( finder.find("\r\n") == 2 );

	REQUIRE( finder.find("\r\na") == std::string::npos );
	REQUIRE( finder.find("\r\n") == std::string::npos );
	REQUIRE( finder.find("\r\n546546546546546\r\nkldsjfljk\n") == 2 );

	REQUIRE( finder.find("\n\nyeet\n\n") == 2 );

	REQUIRE( finder.find("\r\r\n\r\r\r\r") == std::string::npos );
	REQUIRE( finder.find("\r\n") == 2 );


	REQUIRE( finder.find("\r\ns") == std::string::npos );
	REQUIRE( finder.find("\r\r\n") == std::string::npos );
	REQUIRE( finder.find("\r\r\n") == 3 );

	REQUIRE( finder.find("\ns\nssssdflkajsdflkjs\rsdflkjsdflkj\r\nfsdf") == std::string::npos );
	REQUIRE( finder.find("\n") == std::string::npos );
	REQUIRE( finder.find("\n") == 1 );

};
*/
