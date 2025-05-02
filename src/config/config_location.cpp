// Copyright 2025 Ideal Broccoli

#include <string>
#include <vector>

#include "config_http.hpp"
#include "config_location.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

LocationConfig::LocationConfig() :
ConfigBase(),
path(DEFAULT_LOCATION_PATH),
return_value(),
_parent_server(NULL),
_parent_location(NULL) {
}

LocationConfig::LocationConfig(const LocationConfig& other) :
ConfigBase(other),
path(other.path),
return_value(other.return_value),
locations(),
_parent_server(other._parent_server),
_parent_location(other._parent_location) {
}

LocationConfig::~LocationConfig() {
}

void LocationConfig::setParent(ServerConfig* config_server) {
    _parent_server = config_server;
}

ServerConfig* LocationConfig::getServerParent() const {
    return _parent_server;
}

void LocationConfig::setParent(LocationConfig* config_location) {
    _parent_location = config_location;
}

LocationConfig* LocationConfig::getLocationParent() const {
    return _parent_location;
}

void LocationConfig::addLocation(const LocationConfig& location) {
    toolbox::SharedPtr<LocationConfig> new_location(new LocationConfig(location));
    new_location->setParent(this);
    locations.push_back(new_location);
}

const std::vector<toolbox::SharedPtr<LocationConfig> >& LocationConfig::getLocations() const {
    return locations;
}

}  // namespace config
