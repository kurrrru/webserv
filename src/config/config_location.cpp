#include <string>
#include <vector>

#include "config_http.hpp"
#include "config_location.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

LocationConfig::LocationConfig() :
ConfigBase(),
_path(DEFAULT_LOCATION_PATH),
_returnValue(),
_parentServer(NULL),
_parentLocation(NULL) {
}

LocationConfig::LocationConfig(const LocationConfig& other) :
ConfigBase(other),
_path(other._path),
_returnValue(other._returnValue),
_parentServer(other._parentServer),
_parentLocation(other._parentLocation) {
    for (size_t i = 0; i < other._locations.size(); ++i) {
        toolbox::SharedPtr<LocationConfig> newLocation(new LocationConfig(*other._locations[i]));
        _locations.push_back(newLocation);
        _locations.back()->setLocationParent(this);
    }
}

LocationConfig::~LocationConfig() {
}

}  // namespace config
