#include "catch_amalgamated.hpp"

#include "HTTP.hpp"

TEST_CASE( "HTTP capitalizeFieldPretty", "[HTTP]") {

	REQUIRE( HTTP::capitalizeFieldPretty("hello") == "Hello" );
	// REQUIRE( HTTP::capitalizeFieldPretty("") == "" ); // this MIGHT segfault

	REQUIRE( HTTP::capitalizeFieldPretty("content-length") == "Content-Length" );
	REQUIRE( HTTP::capitalizeFieldPretty("-") == "-" );
	REQUIRE( HTTP::capitalizeFieldPretty("-d") == "-D" );
	REQUIRE( HTTP::capitalizeFieldPretty("-hello-world") == "-Hello-World" );
	REQUIRE( HTTP::capitalizeFieldPretty("-hello-") == "-Hello-" );

}


TEST_CASE( "HTTP addHeader", "[HTTP]") {

	HTTP h;

	h.addHeader("YeEt", "amogus");
	REQUIRE( h.getHeader("yeet") == "amogus" );
	h.addHeader("hi", "minecraft");
	REQUIRE( h.getHeader("hi") == "minecraft" );

}

TEST_CASE( "HTTP getHeadersAsString", "[HTTP]") {

	HTTP h;

	REQUIRE( h.getHeadersAsString() == "" );
	h.addHeader("YeEt", "amogus");
	h.addHeader("hi", "mINEcraft");
	h.addHeader("length", "22222");
	REQUIRE( h.getHeadersAsString() == "Hi: mINEcraft\r\nLength: 22222\r\nYeet: amogus\r\n" );

}
