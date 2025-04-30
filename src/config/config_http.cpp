// Copyright 2025 Ideal Broccoli

#include <string>
#include <vector>

#include "config_http.hpp"

namespace config {

HttpConfig::HttpConfig() {
}

HttpConfig::HttpConfig(const HttpConfig& other) : ConfigBase(other) {
    for (std::vector<toolbox::SharedPtr<ServerConfig> >::const_iterator it = other.servers.begin();
         it != other.servers.end(); ++it) {
        toolbox::SharedPtr<ServerConfig> server_copy(new ServerConfig(**it));
        servers.push_back(server_copy);
        servers.back()->setParent(this);
    }
}

HttpConfig::~HttpConfig() {}

void HttpConfig::addServer(const ServerConfig& server) {
    toolbox::SharedPtr<ServerConfig> new_server(new ServerConfig(server));
    servers.push_back(new_server);
    servers.back()->setParent(this);
}

const std::vector<toolbox::SharedPtr<ServerConfig> >& HttpConfig::getServers() const {
    return servers;
}

}  // namespace config
