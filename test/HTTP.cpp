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


TEST_CASE( "HTTP parseHeader", "[HTTP]") {

	HTTP h;

	REQUIRE_NOTHROW( h.parseHeader("derp: yeet") );
	REQUIRE( h.getHeader("derp") == "yeet" );

	REQUIRE_THROWS( h.parseHeader("derp: yeet") );

	REQUIRE_NOTHROW( h.parseHeader("hallo:wereld") );
	REQUIRE( h.getHeader("hallo") == "wereld" );

	REQUIRE_NOTHROW( h.parseHeader("hallo2:  \t    wereld        \t  ") );
	REQUIRE( h.getHeader("hallo2") == "wereld" );

	REQUIRE_NOTHROW( h.parseHeader("h:  \t    wereld") );
	REQUIRE( h.getHeader("h") == "wereld" );

	REQUIRE_NOTHROW( h.parseHeader("h2:wereld \t\t ") );
	REQUIRE( h.getHeader("h2") == "wereld" );

	REQUIRE_THROWS_MATCHES( h.parseHeader("h yeet"), HTTP::ServerException, Catch::Matchers::Message("Header field must end in ':' : h yeet"));

	REQUIRE_THROWS_MATCHES( h.parseHeader("h : yeet"), HTTP::ServerException, Catch::Matchers::Message("Header field is an invalid HTTP token: h "));

	REQUIRE_THROWS_MATCHES( h.parseHeader("h[: yeet"), HTTP::ServerException, Catch::Matchers::Message("Header field is an invalid HTTP token: h["));

	REQUIRE_THROWS_MATCHES( h.parseHeader("asdfasdf"), HTTP::ServerException, Catch::Matchers::Message("Header field must end in ':' : asdfasdf"));

	REQUIRE_NOTHROW( h.parseHeader("hello: ") );
	REQUIRE( h.getHeader("hello") == "" );

	REQUIRE_NOTHROW( h.parseHeader("hello2:") );
	REQUIRE( h.getHeader("hello2") == "" );

	REQUIRE_NOTHROW( h.parseHeader("hello3:        ") );
	REQUIRE( h.getHeader("hello3") == "" );

	REQUIRE_THROWS_MATCHES( h.parseHeader("  hello4: hi"), HTTP::ServerException, Catch::Matchers::Message("Header field is an invalid HTTP token:   hello4"));

	REQUIRE_THROWS_MATCHES( h.parseHeader("h[: yeet"), HTTP::ServerException, Catch::Matchers::Message("Header field is an invalid HTTP token: h["));

	REQUIRE_THROWS_MATCHES( h.parseHeader(": yeet"), HTTP::ServerException, Catch::Matchers::Message("Header field is an invalid HTTP token: "));

}
