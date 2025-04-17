// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include "config_namespace.hpp"

namespace config {

class Config {
 public:
    Config();
    ~Config();

    // 設定の初期化用コンストラクタ (パーサーから使用)
    explicit Config(const config::HttpConfig& config) : _config(config) {}

    const config::HttpConfig& getConfig() const;
    config::HttpConfig& getMutableConfig();
    void setConfig(const config::HttpConfig& config);
    size_t getTokenCount() const;
    void setTokenCount(const size_t token_count);

 private:
    config::HttpConfig _config;
    size_t _token_count;
    Config(const Config& other);
    Config& operator=(const Config& other);

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
