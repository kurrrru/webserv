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
_parent_server(NULL),
_parent_location(NULL) {
}

LocationConfig::~LocationConfig() {
    _parent_location = NULL;
    _parent_server = NULL;
    locations.clear();
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
    new_location->allowed_methods = location.allowed_methods;
    new_location->autoindex = location.autoindex;
    new_location->cgi_extensions = location.cgi_extensions;
    new_location->cgi_pass = location.cgi_pass;
    new_location->client_max_body_size = location.client_max_body_size;
    new_location->error_pages = location.error_pages;
    new_location->root = location.root;
    new_location->indices = location.indices;
    new_location->upload_store = location.upload_store;
    new_location->path = location.path;
    new_location->return_value = location.return_value;
    new_location->setParent(this);
    locations.push_back(new_location);
}

const std::vector<toolbox::SharedPtr<LocationConfig> >& LocationConfig::getLocations() const {
    return locations;
}

}  // namespace config
