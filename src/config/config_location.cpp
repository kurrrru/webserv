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
_parent_server(NULL),
_parent_location(NULL) {
    for (size_t i = 0; i < other._locations.size(); ++i) {
        toolbox::SharedPtr<LocationConfig> new_location(new LocationConfig(*other._locations[i]));
        _locations.push_back(new_location);
        _locations.back()->setLocationParent(this);
    }
}

LocationConfig::~LocationConfig() {
}

}  // namespace config
