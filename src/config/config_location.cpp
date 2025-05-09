#include <string>
#include <vector>

#include "config_http.hpp"
#include "config_location.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

LocationConfig::LocationConfig() :
ConfigBase(),
_path(DEFAULT_LOCATION_PATH),
_return_value(),
_parent_server(NULL),
_parent_location(NULL) {
}

LocationConfig::LocationConfig(const LocationConfig& other) :
ConfigBase(other),
_path(other._path),
_return_value(other._return_value),
_locations(),
_parent_server(other._parent_server),
_parent_location(other._parent_location) {
}

LocationConfig::~LocationConfig() {
}

}  // namespace config
