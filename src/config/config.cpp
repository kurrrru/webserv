#include <string>

#include "config.hpp"
#include "config_http.hpp"
#include "config_parser.hpp"
#include "config_util.hpp"

#include "../../toolbox/shared.hpp"
#include "../../toolbox/stepmark.hpp"

namespace config {

Config::Config() :
_tokenCount(0) {
}

Config::~Config() {
}

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

/**
 * @brief Loads server configuration from the specified file
 * 
 * This static method parses the given configuration file using ConfigParser
 * and applies the resulting settings to the singleton Config instance.
 * If parsing fails, a ConfigException is thrown with an appropriate error message.
 * 
 * @param configFile Path to the configuration file to be loaded
 * @throws ConfigException If the configuration file cannot be parsed successfully
 */
void Config::loadConfig(const std::string& configFile) {
    Config& instance = getInstance();
    ConfigParser parser;
    toolbox::SharedPtr<HttpConfig> httpConfig = parser.parseFile(configFile);
    if (!httpConfig) {
        throwConfigError("Failed to parse configuration file: " + configFile);
    }
    instance._httpConfig = httpConfig;
    instance._tokenCount = parser.getTokenCount();
    toolbox::logger::StepMark::info("Configuration loaded successfully");
}

ConfigException::ConfigException(const std::string& message):
_message(message) {
}

ConfigException::~ConfigException() throw() {
}

const char* ConfigException::what() const throw() {
    return _message.c_str();
}

}  // namespace config
