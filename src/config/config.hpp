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

class Config {
 public:
    Config();
    ~Config();

    const toolbox::SharedPtr<config::HttpConfig>& getHttpConfig() const { return _http_config; }
    void setHttpConfig(const toolbox::SharedPtr<config::HttpConfig>& http_config) { _http_config = http_config; }
    size_t getTokenCount() const { return _token_count; }
    void setTokenCount(const size_t token_count) { _token_count = token_count; }

 private:
    toolbox::SharedPtr<config::HttpConfig> _http_config;
    Config& operator=(const Config& other);
    size_t _token_count;
    Config(const Config& other);
};

class ConfigException : public std::exception {
 public:
    explicit ConfigException(const std::string& message);
    virtual ~ConfigException() throw();
    virtual const char* what() const throw();

 private:
    std::string _message;
};

}  // namespace config
