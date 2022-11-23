#include "catch_amalgamated.hpp"
#include "ConfigParser.hpp"
#include "Listener.hpp"
#include "logger.hpp"

unsigned int    try_invalid_config(std::string path)
{
    unsigned int caught_exceptions = 0;
    std::vector<Listener> listeners;
    try{
        listeners = initFromConfig(path.c_str());
    } catch (std::exception &e) {
        caught_exceptions++;
    }
    return (caught_exceptions);
}

TEST_CASE( "Configs", "[Config]")
{
    std::string config_root_dir = "./test/Config/conf_files/";
    std::vector<Listener> listeners;
    // Valid config
    listeners = initFromConfig( (config_root_dir + "valid_configs/valid1.conf").c_str());
    REQUIRE(!listeners.empty());
    // Error page is a number larger than max int
    // Error page is a negative number


    // Negative port
    REQUIRE(try_invalid_config(config_root_dir + "invalid_configs/invalid1.conf") == 1);
    // Negative CMB in server
    REQUIRE(try_invalid_config(config_root_dir + "invalid_configs/invalid2.conf") == 1);
    // Negative CMB in location
    REQUIRE(try_invalid_config(config_root_dir + "invalid_configs/invalid3.conf") == 1);
    // Negative Error Page in server
    REQUIRE(try_invalid_config(config_root_dir + "invalid_configs/invalid4.conf") == 1);
    // Negative Error Page in location
    REQUIRE(try_invalid_config(config_root_dir + "invalid_configs/invalid5.conf") == 1);
}