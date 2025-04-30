// Copyright 2025 Ideal Broccoli

#include <string>

#include "config.hpp"
#include "config_http.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

Config::Config() :
_token_count(0) {
}

Config::~Config() {}

const toolbox::SharedPtr<config::HttpConfig>& Config::getHttpConfig() const {
    return _http_config;
}

void Config::setHttpConfig(const toolbox::SharedPtr<config::HttpConfig>& http_config) {
    _http_config = http_config;
}

size_t Config::getTokenCount() const {
    return _token_count;
}

void Config::setTokenCount(const size_t token_count) {
    _token_count = token_count;
}

ConfigException::ConfigException(const std::string& message): _message(message) {}

ConfigException::~ConfigException() throw() {}

const char* ConfigException::what() const throw() {
    return _message.c_str();
}

}  // namespace config
