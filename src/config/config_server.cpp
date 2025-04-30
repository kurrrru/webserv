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
listen(),
server_names(),
return_value(),
locations(),
_parent(NULL) {
}

ServerConfig::ServerConfig(const ServerConfig& other) :
ConfigBase(other),
listen(other.listen),
server_names(other.server_names),
return_value(other.return_value),
_parent(NULL) {
    for (std::vector<toolbox::SharedPtr<LocationConfig> >::const_iterator it = other.locations.begin();
        it != other.locations.end(); ++it) {
        toolbox::SharedPtr<LocationConfig> new_location(new LocationConfig(**it));
        locations.push_back(new_location);
        locations.back()->setParent(this);
    }
}

ServerConfig::~ServerConfig() {}

void ServerConfig::setParent(HttpConfig* config_http) {
    _parent = config_http;
}

HttpConfig* ServerConfig::getParent() const {
    return _parent;
}

void ServerConfig::addLocation(const LocationConfig& location) {
    toolbox::SharedPtr<LocationConfig> new_location(new LocationConfig(location));
    locations.push_back(new_location);
    locations.back()->setParent(this);
}

const std::vector<toolbox::SharedPtr<LocationConfig> >& ServerConfig::getLocations() const {
    return locations;
}

}  // namespace config
