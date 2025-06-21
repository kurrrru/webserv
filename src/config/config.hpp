#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include "config_namespace.hpp"
#include "config_http.hpp"
#include "config_server.hpp"
#include "config_location.hpp"


#include "../../toolbox/shared.hpp"

namespace config {
/**
 * @class Config
 * @brief Singleton class for managing server configuration
 *
 * This class implements the Singleton design pattern to provide centralized access
 * to server configuration settings throughout the application. The configuration
 * is loaded from a specified file and parsed into a structured representation.
 *
 * Key features:
 * - Singleton pattern ensures only one configuration instance exists
 * - Loading and parsing of configuration files
 * - Global access to HTTP configuration
 *
 * Usage example:
 * @code
 *   // Load configuration file
 *   config::Config::loadConfig("server.conf");
 *   
 *   // Get singleton instance
 *   const config::Config& config1 = config::Config::getConfig();
 *   
 *   // Or access directly using static method
 *   const toolbox::SharedPtr<config::HttpConfig>& httpConfig = config::Config::getHttpConfig();
 *   
 *   // Use configuration values
 *   std::string root = httpConfig->getRoot();
 *   
 * @endcode
 *
 * @note In C++98 environments, thread safety is not guaranteed. For multi-threaded
 *       applications, it is recommended to load the configuration before creating threads.
 */
class Config {
 public:
    static void loadConfig(const std::string& configFile);
    static Config& getConfig() { return getInstance(); }
    static const toolbox::SharedPtr<config::HttpConfig>&
                    getHttpConfig() { return getInstance()._httpConfig; }
    static std::size_t getTokenCount() { return getInstance()._tokenCount; }
    static void setHttpConfig
    (const toolbox::SharedPtr<config::HttpConfig>& httpConfig) {
        getInstance()._httpConfig = httpConfig;
    }
    static void setTokenCount(const std::size_t tokenCount) {
        getInstance()._tokenCount = tokenCount;
    }

 private:
    Config();
    ~Config();
    Config(const Config& other);
    Config& operator=(const Config& other);

    toolbox::SharedPtr<config::HttpConfig> _httpConfig;
    std::size_t _tokenCount;

    static Config& getInstance();
};

class ConfigException : public std::exception {
 public:
    explicit ConfigException(const std::string& message);
    virtual ~ConfigException() throw();
    ConfigException(const ConfigException& other);
    virtual const char* what() const throw();

 private:
    ConfigException();
    ConfigException& operator=(const ConfigException& other);

    std::string _message;
};

}  // namespace config
