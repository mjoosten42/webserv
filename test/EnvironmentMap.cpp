#include "catch_amalgamated.hpp"

#include "EnvironmentMap.hpp"

TEST_CASE( "EnvironmentMap toCharpp", "[EnvironmentMap]" ) {
	EnvironmentMap ev;

	ev["NAME"] = "yeet";
	ev["VERSION"] = "1.2";

	char **envp = ev.toCharpp();

	REQUIRE(std::string(envp[0]) == "NAME=yeet");
	REQUIRE(std::string(envp[1]) == "VERSION=1.2");
	REQUIRE(envp[2] == nullptr);
	free(envp[0]);
	free(envp[1]);
	free(envp[2]);
	free(envp);
}
