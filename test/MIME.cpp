#include "catch_amalgamated.hpp"

#include "MIME.hpp"

TEST_CASE( "mime", "[MIME]" ) {
	REQUIRE( MIME::fromFileName("derp.txt") == "text/plain" );
	REQUIRE( MIME::fromFileName("derp.css") == "text/css" );
	REQUIRE( MIME::fromFileName(".html") == "text/html" );
	REQUIRE( MIME::fromFileName("html") == "text/plain" );
	REQUIRE( MIME::fromFileName("jeff.asdfasdfadsf") == "text/plain" );
	REQUIRE( MIME::fromFileName("jeff.\1") == "text/plain" );
	REQUIRE( MIME::fromFileName("jeff.\255") == "text/plain" );
	REQUIRE( MIME::fromFileName("jeff.txta") == "text/plain" );
	REQUIRE( MIME::fromFileName("jeff.tar.gz") == "application/gzip" );
	REQUIRE( MIME::fromFileName("jeff.zip") == "application/zip" );
	REQUIRE( MIME::fromFileName("jeff.*3gpp") == "audio/3gpp" );
	REQUIRE( MIME::fromFileName("") == "text/plain" );
}
