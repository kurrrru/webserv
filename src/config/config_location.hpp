#pragma once

#include <string>
#include <vector>

#include "config_namespace.hpp"
#include "config_http.hpp"
#include "config_server.hpp"

#include "../../toolbox/shared.hpp"


namespace config {

class HttpConfig;
class ServerConfig;
/**
 * @class LocationConfig
 * @brief Class for managing location-specific configuration settings
 *
 * This class extends ConfigBase to provide path-specific configuration capabilities.
 * Each instance represents a URL location within a server that can have its own
 * configuration settings. Locations can be nested to form a hierarchy.
 * 
 * LocationConfig inherits common configuration properties from ConfigBase and maintains
 * references to its parent server and/or parent location for inheritance of settings.
 *
 * Usage example:
 * @code
 * // Accessing location configuration settings
 * const std::string& path = location->getPath();                // Returns the URL path (e.g., "/images")
 * const Return& returnValue = location->getReturnValue();       // Gets any configured redirect
 * const std::vector<toolbox::SharedPtr<LocationConfig> >& childLocations = location->getLocations();
 * const ServerConfig* parentServer = location->getServerParent();
 * const LocationConfig* parentLocation = location->getLocationParent();
 * @endcode
 */
class LocationConfig : public ConfigBase {
 public:
    LocationConfig();
    LocationConfig(const LocationConfig&);
    virtual ~LocationConfig();

    void setServerParent(const ServerConfig* config_server) { _parent_server = config_server; }
    void setLocationParent(const LocationConfig* config_location) { _parent_location = config_location; }
    const ServerConfig* getServerParent() const { return _parent_server; }
    const LocationConfig* getLocationParent() const { return _parent_location; }
    const std::string& getPath() const { return _path; }
    void setPath(const std::string& path) { _path = path; }
    const Return& getReturnValue() const { return _return_value; }
    void setReturnValue(const Return& return_value) { _return_value = return_value; }
    const std::vector<toolbox::SharedPtr<LocationConfig> >& getLocations() const { return _locations; }
    void addLocation(const toolbox::SharedPtr<LocationConfig>& location) { _locations.push_back(location); }
    bool hasLocations() const { return !_locations.empty(); }
    size_t getLocationsCount() const { return _locations.size(); }

 private:
    LocationConfig& operator=(const LocationConfig&);

    std::string _path;
    Return _return_value;
    std::vector<toolbox::SharedPtr<LocationConfig> > _locations;
    const ServerConfig* _parent_server;
    const LocationConfig* _parent_location;
};

}  // namespace config
