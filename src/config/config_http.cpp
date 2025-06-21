// Copyright 2025 Ideal Broccoli

#include <string>
#include <vector>

#include "config_http.hpp"

namespace config {

HttpConfig::HttpConfig() {
}

HttpConfig::HttpConfig(const HttpConfig& other) : ConfigBase(other) {
    for (std::size_t i = 0; i < other._servers.size(); ++i) {
        toolbox::SharedPtr<ServerConfig> newServer(new ServerConfig(*other._servers[i].get()));
        _servers.push_back(newServer);
        _servers.back()->setHttpParent(this);
    }
}

HttpConfig::~HttpConfig() {
}

}  // namespace config
