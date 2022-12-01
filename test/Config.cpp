#include "catch_amalgamated.hpp"
#include "ConfigParser.hpp"
#include "Listener.hpp"
#include "logger.hpp"
#include "utils.hpp"

size_t  try_invalid_config(const std::string& path)
{
    size_t caught_exceptions = 0;
    std::vector<Listener> listeners;
    try{
        listeners = initFromConfig(path.c_str());
    } catch (std::exception &e) {
        caught_exceptions++;
    }
    return (caught_exceptions);
}

void    test_all_invalid(std::string invalid_dir)
{
    // Numeric vals - negatives
    // Negative port
    REQUIRE(try_invalid_config(invalid_dir + "invalid1.conf") == 1);
    // Negative CMB in server
    REQUIRE(try_invalid_config(invalid_dir + "invalid2.conf") == 1);
    // Negative CMB in location
    REQUIRE(try_invalid_config(invalid_dir + "invalid3.conf") == 1);
    // Negative Error Page in server
    REQUIRE(try_invalid_config(invalid_dir + "invalid4.conf") == 1);
    // Negative Error Page in location
    REQUIRE(try_invalid_config(invalid_dir + "invalid5.conf") == 1);

    // Numeric vals - overflows
    // Port larger than max ushort
    REQUIRE(try_invalid_config(invalid_dir + "invalid6.conf") == 1);
    // CMB larger than max size_t in server
    REQUIRE(try_invalid_config(invalid_dir + "invalid7.conf") == 1);
    // CMB larger than max size_t in location
    REQUIRE(try_invalid_config(invalid_dir + "invalid8.conf") == 1);
    // Error page larger than max uint in location
    REQUIRE(try_invalid_config(invalid_dir + "invalid9.conf") == 1);
    // Error page larger than max uint in server
    REQUIRE(try_invalid_config(invalid_dir + "invalid10.conf") == 1);

    //Invalid brace-pairing
    REQUIRE(try_invalid_config(invalid_dir + "invalid11.conf") == 1);
    REQUIRE(try_invalid_config(invalid_dir + "invalid12.conf") == 1);
    REQUIRE(try_invalid_config(invalid_dir + "invalid13.conf") == 1);
    REQUIRE(try_invalid_config(invalid_dir + "invalid14.conf") == 1);
    REQUIRE(try_invalid_config(invalid_dir + "invalid15.conf") == 1);
    REQUIRE(try_invalid_config(invalid_dir + "invalid16.conf") == 1);
    REQUIRE(try_invalid_config(invalid_dir + "invalid17.conf") == 1);
    REQUIRE(try_invalid_config(invalid_dir + "invalid18.conf") == 1);

    //Missing semicolons
    REQUIRE(try_invalid_config(invalid_dir + "invalid19.conf") == 1);
    REQUIRE(try_invalid_config(invalid_dir + "invalid20.conf") == 1);

	//Nonsense directives
	REQUIRE(try_invalid_config(invalid_dir + "invalid21.conf") == 1);
	REQUIRE(try_invalid_config(invalid_dir + "invalid22.conf") == 1);

	//Directives in the wrong context
	// Location directives in server
	REQUIRE(try_invalid_config(invalid_dir + "invalid23.conf") == 1);
	REQUIRE(try_invalid_config(invalid_dir + "invalid24.conf") == 1);
	REQUIRE(try_invalid_config(invalid_dir + "invalid25.conf") == 1);
	// Server directives in location
	REQUIRE(try_invalid_config(invalid_dir + "invalid26.conf") == 1);
	REQUIRE(try_invalid_config(invalid_dir + "invalid27.conf") == 1);

	//Location block name does not start with '/'
	REQUIRE(try_invalid_config(invalid_dir + "invalid28.conf") == 1);
	REQUIRE(try_invalid_config(invalid_dir + "invalid29.conf") == 1); // do we want to allow empty location block? treat as root? or throw error?
    return;
}

void    test_all_valid(std::string valid_dir)
{
    std::vector<Listener> listeners;

	#pragma region single_server
	{
    // Random valid config
    listeners = initFromConfig( (valid_dir + "valid1.conf").c_str());
    REQUIRE(!listeners.empty());
    // Default config
    listeners = initFromConfig("./default.conf");
    REQUIRE(!listeners.empty());
    // Limit testing (max vals for port, cmb, error pages etc, multi-line directives, the whole shebang)
    listeners = initFromConfig( (valid_dir + "valid2.conf").c_str());
    Server first = (listeners.front()).getServerByHost("");

    // Location Index
    REQUIRE(first.getLocationIndex("") == 0);
	int A = first.getLocationIndex("/locationA/");
    REQUIRE(A == 2);
	int B = first.getLocationIndex("/locationB/");
    REQUIRE( B == 1);
	int inheritor = first.getLocationIndex("/inheritsFromServer");
	REQUIRE( inheritor == 3);
    REQUIRE(first.getLocationIndex("/E/") == 0);
    REQUIRE(first.getLocationIndex("/") == 0);

    //Server only
    REQUIRE(first.getPort() == 32767);

    REQUIRE(*(first.getNames().begin()) == "www.example.org");
    REQUIRE((first.getNames().back()) == "example.org");

    //Server and Location

    REQUIRE(first.getRoot(0) == "iamroot");
    REQUIRE(first.getRoot(inheritor) == "iamroot");
    REQUIRE(first.getRoot(B) == "alsoroot");

    REQUIRE(first.isAutoIndex(0) == true);
    REQUIRE(first.isAutoIndex(inheritor) == true);
    REQUIRE(first.isAutoIndex(B) == false);

    REQUIRE(first.getIndexPage(0) == "my_index.html");
    REQUIRE(first.getIndexPage(inheritor) == "my_index.html");
    REQUIRE(first.getIndexPage(B) == "not_my_index.html");

    REQUIRE(first.getErrorPage(0, 404) == "-5");
    REQUIRE(first.getErrorPage(0, 405) == "2147483647");
    REQUIRE(first.getErrorPage(0, 406) == "99999999999999");
    REQUIRE(first.getErrorPage(0, 407) == "18446744073709551616");
    REQUIRE(first.getErrorPage(0, 2147483647) == "2147483647.html");

    REQUIRE(first.getErrorPage(inheritor, 404) == "-5");
    REQUIRE(first.getErrorPage(inheritor, 405) == "2147483647");
    REQUIRE(first.getErrorPage(inheritor, 406) == "99999999999999");
    REQUIRE(first.getErrorPage(inheritor, 407) == "18446744073709551616");
    REQUIRE(first.getErrorPage(inheritor, 2147483647) == "2147483647.html");

    REQUIRE(first.getErrorPage(A, 504) == "-5");
    REQUIRE(first.getErrorPage(A, 505) == "2147483647");
    REQUIRE(first.getErrorPage(A, 506) == "99999999999999");
    REQUIRE(first.getErrorPage(A, 507) == "18446744073709551616");
    REQUIRE(first.getErrorPage(A, 2147483647) == "2147483647.html");

    REQUIRE(first.hasErrorPage(0, 504) == false);
    REQUIRE(first.hasErrorPage(inheritor, 504) == false);
    REQUIRE(first.hasErrorPage(A, 404) == false);

    size_t max_cmb = 18446744073709551615U;
    REQUIRE(first.getCMB(0) == max_cmb); //server
    REQUIRE(first.getCMB(inheritor) == max_cmb); //location that inherits
    REQUIRE(first.getCMB(A) == max_cmb); //location that sets its own cmb
    REQUIRE(first.getCMB(B) == 0); //location that sets its own cmb


    REQUIRE(first.getUploadDir(0) == "amogus_storage");
    REQUIRE(first.getUploadDir(inheritor) == "amogus_storage");
    REQUIRE(first.getUploadDir(A) == "amogus_storage");
    REQUIRE(first.getUploadDir(B) == "not_amogus_storage");

    //Location only

    REQUIRE(first.isRedirect(0) == false);
    REQUIRE(first.isRedirect(inheritor) == false);
    REQUIRE(first.isRedirect(A) == false);
    REQUIRE(first.isRedirect(B) == true);
    REQUIRE(first.getRedirect(B) == "/locationA");
    
    REQUIRE(first.isCGI(0, "pl") == false);
    REQUIRE(first.isCGI(inheritor, "pl") == false);
    REQUIRE(first.isCGI(A, "pl") == true);
    REQUIRE(first.isCGI(A, "p") == false);
    REQUIRE(first.isCGI(A, "pll") == false);

    REQUIRE(first.getAllowedMethodsAsString(0) == "{ GET, POST, DELETE }");
    REQUIRE(first.getAllowedMethodsAsString(A) == "{ POST, DELETE }");
    REQUIRE(first.getAllowedMethodsAsString(B) == "{ GET }");
	}
	#pragma endregion

	#pragma region multiple_servers
	{
		// Limit testing (max vals for port, cmb, error pages etc, multi-line directives, the whole shebang)
		listeners = initFromConfig( (valid_dir + "valid3.conf").c_str());
		const Server &first = (listeners.front()).getServerByHost("");
		const Server &second = (listeners.front()).getServerByHost("job.com");
		const Server &third = (listeners.back()).getServerByHost("job.com");

		// Location Index
		REQUIRE(first.getLocationIndex("") == 0);
		int A = first.getLocationIndex("/locationA/");
		REQUIRE(A == 2); // 4
		int B = first.getLocationIndex("/locationB/");
		REQUIRE( B == 1);
		int inheritor = first.getLocationIndex("/inheritsFromServer");
		REQUIRE( inheritor == 3); //7
		REQUIRE(first.getLocationIndex("/E/") == 0);
		REQUIRE(first.getLocationIndex("/") == 0);

		//Server only
		REQUIRE(first.getPort() == 32767);

		REQUIRE(*(first.getNames().begin()) == "www.example.org");
		REQUIRE((first.getNames().back()) == "example.org");

		//Server and Location

		REQUIRE(first.getRoot(0) == "iamroot");
		REQUIRE(first.getRoot(inheritor) == "iamroot");
		REQUIRE(first.getRoot(B) == "alsoroot");

		REQUIRE(first.isAutoIndex(0) == true);
		REQUIRE(first.isAutoIndex(inheritor) == true);
		REQUIRE(first.isAutoIndex(B) == false);

		REQUIRE(first.getIndexPage(0) == "my_index.html");
		REQUIRE(first.getIndexPage(inheritor) == "my_index.html");
		REQUIRE(first.getIndexPage(B) == "not_my_index.html");

		REQUIRE(first.getErrorPage(0, 404) == "-5");
		REQUIRE(first.getErrorPage(0, 405) == "2147483647");
		REQUIRE(first.getErrorPage(0, 406) == "99999999999999");
		REQUIRE(first.getErrorPage(0, 407) == "18446744073709551616");
		REQUIRE(first.getErrorPage(0, 2147483647) == "2147483647.html");

		REQUIRE(first.getErrorPage(inheritor, 404) == "-5");
		REQUIRE(first.getErrorPage(inheritor, 405) == "2147483647");
		REQUIRE(first.getErrorPage(inheritor, 406) == "99999999999999");
		REQUIRE(first.getErrorPage(inheritor, 407) == "18446744073709551616");
		REQUIRE(first.getErrorPage(inheritor, 2147483647) == "2147483647.html");

		REQUIRE(first.getErrorPage(A, 504) == "-5");
		REQUIRE(first.getErrorPage(A, 505) == "2147483647");
		REQUIRE(first.getErrorPage(A, 506) == "99999999999999");
		REQUIRE(first.getErrorPage(A, 507) == "18446744073709551616");
		REQUIRE(first.getErrorPage(A, 2147483647) == "2147483647.html");

		REQUIRE(first.hasErrorPage(0, 504) == false);
		REQUIRE(first.hasErrorPage(inheritor, 504) == false);
		REQUIRE(first.hasErrorPage(A, 404) == false);

		size_t max_cmb = 18446744073709551615U;
		REQUIRE(first.getCMB(0) == max_cmb); //server
		REQUIRE(first.getCMB(inheritor) == max_cmb); //location that inherits
		REQUIRE(first.getCMB(A) == max_cmb); //location that sets its own cmb
		REQUIRE(first.getCMB(B) == 0); //location that sets its own cmb


		REQUIRE(first.getUploadDir(0) == "amogus_storage");
		REQUIRE(first.getUploadDir(inheritor) == "amogus_storage");
		REQUIRE(first.getUploadDir(A) == "amogus_storage");
		REQUIRE(first.getUploadDir(B) == "not_amogus_storage");

		//Location only

		REQUIRE(first.isRedirect(0) == false);
		REQUIRE(first.isRedirect(inheritor) == false);
		REQUIRE(first.isRedirect(A) == false);
		REQUIRE(first.isRedirect(B) == true);
		REQUIRE(first.getRedirect(B) == "/locationA");
		
		REQUIRE(first.isCGI(0, "pl") == false);
		REQUIRE(first.isCGI(inheritor, "pl") == false);
		REQUIRE(first.isCGI(A, "pl") == true);
		REQUIRE(first.isCGI(A, "p") == false);
		REQUIRE(first.isCGI(A, "pll") == false);

		REQUIRE(first.getAllowedMethodsAsString(0) == "{ GET, POST, DELETE }");
		REQUIRE(first.getAllowedMethodsAsString(A) == "{ POST, DELETE }");
		REQUIRE(first.getAllowedMethodsAsString(B) == "{ GET }");


		//second
		REQUIRE(second.getLocationIndex("") == 0);
		A = second.getLocationIndex("/locationA/");
		REQUIRE(A == 2); //3
		B = second.getLocationIndex("/locationB/");
		REQUIRE( B == 1);
		inheritor = second.getLocationIndex("/inheritsFromServer");
		REQUIRE( inheritor == 3); //5
		REQUIRE(second.getLocationIndex("/E/") == 0);
		REQUIRE(second.getLocationIndex("/") == 0);

		//Server only
		REQUIRE(second.getPort() == 32767);

		REQUIRE(*(second.getNames().begin()) == "job.com");
		REQUIRE((second.getNames().back()) == "job.com");

		//Server and Location

		REQUIRE(second.getRoot(0) == "iamroot");
		REQUIRE(second.getRoot(inheritor) == "iamroot");
		REQUIRE(second.getRoot(B) == "alsoroot");

		REQUIRE(second.isAutoIndex(0) == true);
		REQUIRE(second.isAutoIndex(inheritor) == true);
		REQUIRE(second.isAutoIndex(B) == false);

		REQUIRE(second.getIndexPage(0) == "my_index.html");
		REQUIRE(second.getIndexPage(inheritor) == "my_index.html");
		REQUIRE(second.getIndexPage(B) == "not_my_index.html");

		REQUIRE(second.getErrorPage(0, 404) == "-5");
		REQUIRE(second.getErrorPage(0, 405) == "2147483647");
		REQUIRE(second.getErrorPage(0, 406) == "99999999999999");
		REQUIRE(second.getErrorPage(0, 407) == "18446744073709551616");
		REQUIRE(second.getErrorPage(0, 2147483647) == "2147483647.html");

		REQUIRE(second.getErrorPage(inheritor, 404) == "-5");
		REQUIRE(second.getErrorPage(inheritor, 405) == "2147483647");
		REQUIRE(second.getErrorPage(inheritor, 406) == "99999999999999");
		REQUIRE(second.getErrorPage(inheritor, 407) == "18446744073709551616");
		REQUIRE(second.getErrorPage(inheritor, 2147483647) == "2147483647.html");

		REQUIRE(second.getErrorPage(A, 504) == "-5");
		REQUIRE(second.getErrorPage(A, 505) == "2147483647");
		REQUIRE(second.getErrorPage(A, 506) == "99999999999999");
		REQUIRE(second.getErrorPage(A, 507) == "18446744073709551616");
		REQUIRE(second.getErrorPage(A, 2147483647) == "2147483647.html");

		REQUIRE(second.hasErrorPage(0, 504) == false);
		REQUIRE(second.hasErrorPage(inheritor, 504) == false);
		REQUIRE(second.hasErrorPage(A, 404) == false);

		max_cmb = 18446744073709551615U;
		REQUIRE(second.getCMB(0) == max_cmb); //server
		REQUIRE(second.getCMB(inheritor) == max_cmb); //location that inherits
		REQUIRE(second.getCMB(A) == max_cmb); //location that sets its own cmb
		REQUIRE(second.getCMB(B) == 0); //location that sets its own cmb


		REQUIRE(second.getUploadDir(0) == "amogus_storage");
		REQUIRE(second.getUploadDir(inheritor) == "amogus_storage");
		REQUIRE(second.getUploadDir(A) == "amogus_storage");
		REQUIRE(second.getUploadDir(B) == "not_amogus_storage");

		//Location only

		REQUIRE(second.isRedirect(0) == false);
		REQUIRE(second.isRedirect(inheritor) == false);
		REQUIRE(second.isRedirect(A) == false);
		REQUIRE(second.isRedirect(B) == true);
		REQUIRE(second.getRedirect(B) == "/locationA");
		
		REQUIRE(second.isCGI(0, "pl") == false);
		REQUIRE(second.isCGI(inheritor, "pl") == false);
		REQUIRE(second.isCGI(A, "pl") == true);
		REQUIRE(second.isCGI(A, "p") == false);
		REQUIRE(second.isCGI(A, "pll") == false);

		REQUIRE(second.getAllowedMethodsAsString(0) == "{ GET, POST, DELETE }");
		REQUIRE(second.getAllowedMethodsAsString(A) == "{ POST, DELETE }");
		REQUIRE(second.getAllowedMethodsAsString(B) == "{ GET }");

		//third
		REQUIRE(third.getLocationIndex("") == 0);
		A = third.getLocationIndex("/locationA/");
		REQUIRE(A == 2);
		B = third.getLocationIndex("/locationB/");
		REQUIRE( B == 1);
		inheritor = third.getLocationIndex("/inheritsFromServer");
		REQUIRE( inheritor == 3);
		REQUIRE(third.getLocationIndex("/E/") == 0);
		REQUIRE(third.getLocationIndex("/") == 0);

		//Server only
		REQUIRE(third.getPort() == 4040);

		REQUIRE(*(third.getNames().begin()) == "job.com");
		REQUIRE((third.getNames().back()) == "job.com");
		//Server and Location

		REQUIRE(third.getRoot(0) == "iamroot");
		REQUIRE(third.getRoot(inheritor) == "iamroot");
		REQUIRE(third.getRoot(B) == "alsoroot");

		REQUIRE(third.isAutoIndex(0) == true);
		REQUIRE(third.isAutoIndex(inheritor) == true);
		REQUIRE(third.isAutoIndex(B) == false);

		REQUIRE(third.getIndexPage(0) == "my_index.html");
		REQUIRE(third.getIndexPage(inheritor) == "my_index.html");
		REQUIRE(third.getIndexPage(B) == "not_my_index.html");

		REQUIRE(third.getErrorPage(0, 404) == "-5");
		REQUIRE(third.getErrorPage(0, 405) == "2147483647");
		REQUIRE(third.getErrorPage(0, 406) == "99999999999999");
		REQUIRE(third.getErrorPage(0, 407) == "18446744073709551616");
		REQUIRE(third.getErrorPage(0, 2147483647) == "2147483647.html");

		REQUIRE(third.getErrorPage(inheritor, 404) == "-5");
		REQUIRE(third.getErrorPage(inheritor, 405) == "2147483647");
		REQUIRE(third.getErrorPage(inheritor, 406) == "99999999999999");
		REQUIRE(third.getErrorPage(inheritor, 407) == "18446744073709551616");
		REQUIRE(third.getErrorPage(inheritor, 2147483647) == "2147483647.html");

		REQUIRE(third.getErrorPage(A, 504) == "-5");
		REQUIRE(third.getErrorPage(A, 505) == "2147483647");
		REQUIRE(third.getErrorPage(A, 506) == "99999999999999");
		REQUIRE(third.getErrorPage(A, 507) == "18446744073709551616");
		REQUIRE(third.getErrorPage(A, 2147483647) == "2147483647.html");

		REQUIRE(third.hasErrorPage(0, 504) == false);
		REQUIRE(third.hasErrorPage(inheritor, 504) == false);
		REQUIRE(third.hasErrorPage(A, 404) == false);

		max_cmb = 18446744073709551615U;
		REQUIRE(third.getCMB(0) == max_cmb); //server
		REQUIRE(third.getCMB(inheritor) == max_cmb); //location that inherits
		REQUIRE(third.getCMB(A) == max_cmb); //location that sets its own cmb
		REQUIRE(third.getCMB(B) == 0); //location that sets its own cmb


		REQUIRE(third.getUploadDir(0) == "amogus_storage");
		REQUIRE(third.getUploadDir(inheritor) == "amogus_storage");
		REQUIRE(third.getUploadDir(A) == "amogus_storage");
		REQUIRE(third.getUploadDir(B) == "not_amogus_storage");

		//Location only

		REQUIRE(third.isRedirect(0) == false);
		REQUIRE(third.isRedirect(inheritor) == false);
		REQUIRE(third.isRedirect(A) == false);
		REQUIRE(third.isRedirect(B) == true);
		REQUIRE(third.getRedirect(B) == "/locationA");
		
		REQUIRE(third.isCGI(0, "pl") == false);
		REQUIRE(third.isCGI(inheritor, "pl") == false);
		REQUIRE(third.isCGI(A, "pl") == true);
		REQUIRE(third.isCGI(A, "p") == false);
		REQUIRE(third.isCGI(A, "pll") == false);

		REQUIRE(third.getAllowedMethodsAsString(0) == "{ GET, POST, DELETE }");
		REQUIRE(third.getAllowedMethodsAsString(A) == "{ POST, DELETE }");
		REQUIRE(third.getAllowedMethodsAsString(B) == "{ GET }");
	}
	#pragma endregion
    
}

TEST_CASE( "Configs", "[Config]")
{
    std::string config_root_dir = "./test/Config/";

	REQUIRE_NOTHROW(test_all_invalid(config_root_dir + "invalid/"));

    test_all_valid(config_root_dir + "valid/");

}
