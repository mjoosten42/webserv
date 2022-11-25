#include "catch_amalgamated.hpp"

#include "Request.hpp"

static void easySend(Request& req, const std::string& raw) {

	req.append(raw.c_str(), raw.length());
}

TEST_CASE( "Request generic", "[Request]") {

	Request r;
	easySend(r, "GET / HTTP/1.1\r\ndERP:  cool\r\nHOST: yeet\r\n\r\n");
	REQUIRE( r.getMethod() == GET );
	REQUIRE( r.getLocation() == "/" );
	REQUIRE( r.getHeader("derp") == "cool" );
	REQUIRE( r.getHost() == "yeet" );
	REQUIRE( r.getState() == DONE );

}

TEST_CASE( "Request LF instead of CRLF", "[Request]") {

	Request r;
	easySend(r, "GET / HTTP/1.1\nHello: world\nHost: yeet\n\n");
	REQUIRE( r.getMethod() == GET );
	REQUIRE( r.getLocation() == "/" );
	REQUIRE( r.getHeader("Hello") == "world" );
	REQUIRE( r.getHost() == "yeet" );
	REQUIRE( r.getState() == DONE );

}

TEST_CASE( "Request header without space", "[Request]") {

	Request r;
	easySend(r, "GET / HTTP/1.1\r\ndERP:cool\r\nHOST: yeet\r\n\r\n");
	REQUIRE( r.getMethod() == GET );
	REQUIRE( r.getLocation() == "/" );
	REQUIRE( r.getHeader("derp") == "cool" );
	REQUIRE( r.getHost() == "yeet" );
	REQUIRE( r.getState() == DONE );

}


TEST_CASE( "Invalid requests", "[Request]") {

	Request r;
	easySend(r, "HELLO / HTTP/1.1\r\ndERP:     cool\r\nHOST: yeet\r\n\r\n");
	REQUIRE( r.getMethod() == INVALID );
	REQUIRE( r.getState() == DONE );
	
}

TEST_CASE( "Request status", "[Request]") {

	Request r;
	easySend(r, "GET ");
	REQUIRE( r.getState() == STARTLINE );
	easySend(r, " /");
	REQUIRE( r.getState() == STARTLINE );
	easySend(r, " HTTP/1.1"); 
	REQUIRE( r.getState() == STARTLINE );
	easySend(r, "\r\ndERP:     cool\r\n");
	REQUIRE( r.getState() == HEADERS);
	easySend(r, "HOST: yeet\r\nContent-Length: 10\r\n\r\nYOOOO");
	REQUIRE( r.getState() == BODY );
	easySend(r, "YOOOO");
	REQUIRE( r.getState() == DONE );

}
