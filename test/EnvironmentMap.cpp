#include "catch_amalgamated.hpp"

#include "EnvironmentMap.hpp"

void free_array(char **array) {
	char** start = array;

	while (*array)
		free(*array++);
	free(start);
}

TEST_CASE( "EnvironmentMap addEnv", "[EnvironmentMap]" ) {
	EnvironmentMap ev;

	ev.addEnv();

	REQUIRE(ev["USER"] != "");
}

TEST_CASE( "EnvironmentMap toCharpp", "[EnvironmentMap]" ) {
	EnvironmentMap ev;

	ev["NAME"] = "yeet";
	ev["VERSION"] = "1.2";

	char **envp = ev.toCharpp();

	REQUIRE(std::string(envp[0]) == "NAME=yeet");
	REQUIRE(std::string(envp[1]) == "VERSION=1.2");
	REQUIRE(envp[2] == nullptr);
	free_array(envp);
}
