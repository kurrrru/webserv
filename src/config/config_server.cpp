// Copyright 2025 Ideal Broccoli

#include <string>
#include <vector>

#include "config_base.hpp"
#include "config_http.hpp"
#include "config_server.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

ServerConfig::ServerConfig() :
ConfigBase(),
_listens(),
_serverNames(),
_returnValue(),
_locations(),
_parent(NULL) {
}

ServerConfig::ServerConfig(const ServerConfig& other) :
ConfigBase(other),
_listens(other._listens),
_serverNames(other._serverNames),
_returnValue(other._returnValue),
_parent(other._parent) {
    for (std::size_t i = 0; i < other._locations.size(); ++i) {
        toolbox::SharedPtr<LocationConfig> newLocation(new LocationConfig(*other._locations[i]));
        _locations.push_back(newLocation);
        _locations.back()->setServerParent(this);
    }
}

ServerConfig::~ServerConfig() {
}

}  // namespace config
