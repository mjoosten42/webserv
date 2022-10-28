#include "catch_amalgamated.hpp"

#include "Request.hpp"

static void easySend(Request& req, const std::string& raw) {

	req.append(raw.c_str(), raw.length());
}

TEST_CASE( "Request generic", "[Request]") {

	Request r;
	easySend(r, "GET / HTTP/1.1\r\ndERP:     cool\r\nHOST: yeet\r\n\r\n");
	REQUIRE( r.getMethod() == GET );
	REQUIRE( r.getLocation() == "/" );
	REQUIRE( r.getHeaderValue("derp") == "cool" );
	REQUIRE( r.getHost() == "yeet" );
}
