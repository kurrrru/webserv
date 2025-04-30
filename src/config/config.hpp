// Copyright 2025 Ideal Broccoli

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

    const toolbox::SharedPtr<config::HttpConfig>& getHttpConfig() const;
    void setHttpConfig(const toolbox::SharedPtr<config::HttpConfig>& config);
    size_t getTokenCount() const;
    void setTokenCount(const size_t token_count);

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
