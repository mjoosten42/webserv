#include "catch_amalgamated.hpp"

#include "Request.hpp"

class TesterHelper {

public:
	void statusLineHelper(std::string input, methods method, std::string location, std::string queryString) {

		Request r;
		r.parseStartLine(input);

		REQUIRE( r.getMethod() == method );
		REQUIRE( r.getLocation() == location );
		REQUIRE( r.getQueryString() == queryString );

	};
};

void statusLineHelper(std::string input, methods method, std::string location, std::string queryString = "") {
	TesterHelper h;
	h.statusLineHelper(input, method, location, queryString);
}

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
	REQUIRE( r.getStateAsString() == "BODY" );
	easySend(r, "YOOOO");
	REQUIRE( r.getState() == DONE );
	REQUIRE( r.getContentLength() == 10 );

}


TEST_CASE( "Request statusline", "[Request]") {

	REQUIRE_NOTHROW( statusLineHelper("GET / HTTP/1.1", GET, "/") );

	REQUIRE_NOTHROW( statusLineHelper("GET /index.html HTTP/1.1", GET, "/index.html") );

	REQUIRE_NOTHROW( statusLineHelper("POST /index.html HTTP/1.1", POST, "/index.html") );

	REQUIRE_NOTHROW( statusLineHelper("POST /index.html?a=b HTTP/1.1", POST, "/index.html", "a=b") );
	REQUIRE_NOTHROW( statusLineHelper("POST /index.html? HTTP/1.1", POST, "/index.html", "") );
	REQUIRE_NOTHROW( statusLineHelper("POST /?a=b HTTP/1.1", POST, "/", "a=b") );

	REQUIRE_THROWS_MATCHES( statusLineHelper("POST /index.html HTTP/1.0", POST, "/index.html"), HTTP::ServerException, Catch::Matchers::Message("HTTP/1.1 only: HTTP/1.0") );

	REQUIRE_THROWS_MATCHES( statusLineHelper("POST /index.html", POST, "/index.html"), HTTP::ServerException, Catch::Matchers::Message("Missing HTTP version") );

	REQUIRE_THROWS_MATCHES( statusLineHelper("REKT /index.html HTTP/1.1", POST, "/index.html"), HTTP::ServerException, Catch::Matchers::Message("Incorrect method: REKT") );

}
