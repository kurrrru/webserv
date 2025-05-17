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
_server_names(),
_return_value(),
_locations(),
_parent(NULL) {
}

ServerConfig::ServerConfig(const ServerConfig& other) :
ConfigBase(other),
_listens(other._listens),
_server_names(other._server_names),
_return_value(other._return_value),
_parent(other._parent) {
    for (size_t i = 0; i < other._locations.size(); ++i) {
        toolbox::SharedPtr<LocationConfig> new_location(new LocationConfig(*other._locations[i]));
        _locations.push_back(new_location);
        _locations.back()->setServerParent(this);
    }
}

ServerConfig::~ServerConfig() {
}

}  // namespace config
