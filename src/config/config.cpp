// Copyright 2025 Ideal Broccoli

#include <string>

#include "config.hpp"

namespace config {

Config::Config() : _token_count(0) {}

Config::~Config() {}

const config::HttpConfig& Config::getConfig() const {
    return _config;
}

config::HttpConfig& Config::getMutableConfig() {
    return _config;
}

void Config::setConfig(const config::HttpConfig& config) {
    _config = config;
}

size_t Config::getTokenCount() const {
    return _token_count;
}

void Config::setTokenCount(const size_t token_count) {
    _token_count = token_count;
}

ConfigException::ConfigException(const std::string& message)
    : _message(message) {}

ConfigException::~ConfigException() throw() {}

const char* ConfigException::what() const throw() {
    return _message.c_str();
}

}  // namespace config
