#pragma once

#include <string>
#include <vector>

#include "config_namespace.hpp"
#include "config_http.hpp"
#include "config_location.hpp"

#include "../../toolbox/shared.hpp"

namespace config {
/**
 * @class ServerConfig
 * @brief Class for managing server configuration settings
 *
 * This class extends ConfigBase to provide server-specific configuration capabilities.
 * Each instance represents a virtual server that can listen on specific IP:port 
 * combinations and respond to requests for specific server names (domains).
 * 
 * ServerConfig inherits common configuration properties from ConfigBase and can
 * contain multiple location configurations for path-specific settings. It also
 * maintains a reference to its parent HttpConfig to access global defaults.
 *
 * Usage example:
 * @code
 * ServerConfig server;
 * 
 * // Copy an existing location with modifications
 * LocationConfig* copyOfLocation = new LocationConfig(*location);
 * toolbox::SharedPtr<LocationConfig> newLocation(copyOfLocation);
 * newLocation->setPath("/api/v2");
 * newLocation->setRoot("/var/www/api/v2");
 * server.addLocation(newLocation);
 * 
 * // Accessing the configured settings
 * const std::vector<Listen>& listens = server.getListens();
 * const std::vector<ServerName>& names = server.getServerNames();
 * const std::vector<toolbox::SharedPtr<LocationConfig> >& locations = server.getLocations();
 * @endcode
 */
class ServerConfig : public ConfigBase {
 public:
    ServerConfig();
    ServerConfig(const ServerConfig&);
    virtual ~ServerConfig();

    void setHttpParent(const HttpConfig* config_http) { _parent = config_http; }
    const HttpConfig* getParent() const { return _parent; }
    const std::vector<Listen>& getListens() const { return _listens; }
    void setListens(const std::vector<Listen>& listens) { _listens = listens; }
    void addListen(const Listen& listen) { _listens.push_back(listen); }
    bool hasListens() const { return !_listens.empty(); }
    const std::vector<ServerName>& getServerNames() const { return _serverNames; }
    void setServerNames(const std::vector<ServerName>& serverNames) { _serverNames = serverNames; }
    void addServerName(const ServerName& serverName) { _serverNames.push_back(serverName); }
    bool hasServerNames() const { return !_serverNames.empty(); }
    const Return& getReturnValue() const { return _returnValue; }
    void setReturnValue(const Return& returnValue) { _returnValue = returnValue; }
    const std::vector<toolbox::SharedPtr<LocationConfig> >& getLocations() const { return _locations; }
    void addLocation(const toolbox::SharedPtr<LocationConfig>& location) { _locations.push_back(location); }
    bool hasLocations() const { return !_locations.empty(); }
    std::size_t getLocationsCount() const { return _locations.size(); }

 private:
    ServerConfig& operator=(const ServerConfig&);

    std::vector<Listen> _listens;
    std::vector<ServerName> _serverNames;
    Return _returnValue;
    std::vector<toolbox::SharedPtr<LocationConfig> > _locations;
    const HttpConfig* _parent;
};

}  // namespace config
