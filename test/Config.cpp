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
    return;
}

void    test_all_valid(std::string valid_dir)
{
    std::vector<Listener> listeners;
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

TEST_CASE( "Configs", "[Config]")
{
    std::string config_root_dir = "./test/Config/";

    test_all_invalid(config_root_dir + "invalid/");
    test_all_valid(config_root_dir + "valid/");

}
