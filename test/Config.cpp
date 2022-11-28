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
    REQUIRE(first.getPort() == 32767);


    REQUIRE(first.getLocationIndex("") == 0);
    REQUIRE(first.getLocationIndex("/locationA/") == 4);
    // REQUIRE(first.getLocationIndex("/locationA") == 4);

    REQUIRE(first.getLocationIndex("/locationB/") == 2);
    // REQUIRE(first.getLocationIndex("/locationB") == 2);

    // REQUIRE(first.getLocationIndex("/locationC/") == 3);
    // REQUIRE(first.getLocationIndex("/locationC") == 3);

    // REQUIRE(first.getLocationIndex("/locationD/") == 4);
    // REQUIRE(first.getLocationIndex("/locationD") == 4);

    REQUIRE(first.getLocationIndex("/E/") == 0);
    // REQUIRE(first.getLocationIndex("/E") == 0);
    REQUIRE(first.getLocationIndex("/") == 0);

    REQUIRE(first.getLocationIndex("/inheritsFromServer/") == 5);

    REQUIRE(first.getErrorPage(0, 404) == "-5");
    REQUIRE(first.getErrorPage(0, 405) == "2147483647");
    REQUIRE(first.getErrorPage(0, 406) == "99999999999999");
    REQUIRE(first.getErrorPage(0, 407) == "18446744073709551616");
    REQUIRE(first.getErrorPage(0, 2147483647) == "2147483647.html");

    REQUIRE(first.getErrorPage(5, 404) == "-5");
    REQUIRE(first.getErrorPage(5, 405) == "2147483647");
    REQUIRE(first.getErrorPage(5, 406) == "99999999999999");
    REQUIRE(first.getErrorPage(5, 407) == "18446744073709551616");
    REQUIRE(first.getErrorPage(5, 2147483647) == "2147483647.html");

    REQUIRE(first.getErrorPage(4, 504) == "-5");
    REQUIRE(first.getErrorPage(4, 505) == "2147483647");
    REQUIRE(first.getErrorPage(4, 506) == "99999999999999");
    REQUIRE(first.getErrorPage(4, 507) == "18446744073709551616");
    REQUIRE(first.getErrorPage(4, 2147483647) == "2147483647.html");

    REQUIRE(*(first.getNames().begin()) == "www.example.org");
    REQUIRE((first.getNames().back()) == "example.org");

    REQUIRE(first.getRoot(0) == "iamroot");
    REQUIRE(first.getRoot(5) == "iamroot");
    REQUIRE(first.getRoot(2) == "alsoroot");

    REQUIRE(first.getIndexPage(0) == "my_index.html");
    REQUIRE(first.getIndexPage(5) == "my_index.html");
    REQUIRE(first.getIndexPage(2) == "not_my_index.html");

    REQUIRE(first.isAutoIndex(0) == true);
    REQUIRE(first.isAutoIndex(5) == true);
    REQUIRE(first.isAutoIndex(2) == false);

    size_t max_cmb = 18446744073709551615U;
    REQUIRE(first.getCMB(0) == max_cmb); //server
    REQUIRE(first.getCMB(5) == max_cmb); //location that inherits
    REQUIRE(first.getCMB(4) == max_cmb); //location that sets its own cmb
    REQUIRE(first.getCMB(2) == 0); //location that sets its own cmb
    
}

TEST_CASE( "Configs", "[Config]")
{
    std::string config_root_dir = "./test/Config/";

    test_all_invalid(config_root_dir + "invalid/");
    test_all_valid(config_root_dir + "valid/");

}
