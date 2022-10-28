#include "catch_amalgamated.hpp"

#include "Request.hpp"

static int easySend(Request& req, const std::string& raw) {

	return req.append(raw.c_str(), raw.length());
}

TEST_CASE( "Request generic", "[Request]") {

	Request r;
	REQUIRE( easySend(r, "GET / HTTP/1.1\r\ndERP:     cool\r\nHOST: yeet\r\n") == 200 );
	REQUIRE( r.getMethod() == GET );
	REQUIRE( r.getLocation() == "/" );
	REQUIRE( r.getHeaderValue("derp") == "cool" );
	REQUIRE( r.getHost() == "yeet" ); // TODO: why does it fail?

	// M: request is still parsing headers and hasn't checked for host yet
	r.clear();
	easySend(r, "GET / HTTP/1.1\r\nHOST: yeet\r\n");
	easySend(r, "\r\n");
	REQUIRE( r.getHost() == "yeet" ); // Does work

}
