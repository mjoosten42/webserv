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
    REQUIRE(first.getPort() == 65535);


    REQUIRE(first.getLocationIndex("") == 0);
    REQUIRE(first.getLocationIndex("/locationA/") == 1);
    REQUIRE(first.getLocationIndex("/locationA") == 1);

    REQUIRE(first.getLocationIndex("/locationB/") == 2);
    REQUIRE(first.getLocationIndex("/locationB") == 2);

    REQUIRE(first.getLocationIndex("/locationC/") == 3);
    REQUIRE(first.getLocationIndex("/locationC") == 3);

    REQUIRE(first.getLocationIndex("/locationD/") == 4);
    REQUIRE(first.getLocationIndex("/locationD") == 4);

    REQUIRE(first.getLocationIndex("/E/") == 0);
    REQUIRE(first.getLocationIndex("/E") == 0);
    REQUIRE(first.getLocationIndex("/") == 0);



    // REQUIRE(first.getErrorPage(0, 404) == "-5");

}

TEST_CASE( "Configs", "[Config]")
{
    std::string config_root_dir = "./test/Config/conf_files/";

    test_all_invalid(config_root_dir + "invalid_configs/");
    test_all_valid(config_root_dir + "valid_configs/");

}
