#include <string>
#include <vector>

#include "config_http.hpp"

namespace config {

HttpConfig::HttpConfig() {
}

HttpConfig::HttpConfig(const HttpConfig& other) : ConfigBase(other) {
    for (size_t i = 0; i < other._servers.size(); ++i) {
        toolbox::SharedPtr<ServerConfig> new_server(new ServerConfig(*other._servers[i].get()));
        _servers.push_back(new_server);
        _servers.back()->setHttpParent(this);
    }
}

HttpConfig::~HttpConfig() {
}

}  // namespace config
