#pragma once

#include <string>
#include <vector>

#include "config_namespace.hpp"
#include "config_http.hpp"
#include "config_location.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

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
    const std::vector<ServerName>& getServerNames() const { return _server_names; }
    void setServerNames(const std::vector<ServerName>& server_names) { _server_names = server_names; }
    void addServerName(const ServerName& server_name) { _server_names.push_back(server_name); }
    bool hasServerNames() const { return !_server_names.empty(); }
    const Return& getReturnValue() const { return _return_value; }
    void setReturnValue(const Return& return_value) { _return_value = return_value; }
    const std::vector<toolbox::SharedPtr<LocationConfig> >& getLocations() const { return _locations; }
    void addLocation(const toolbox::SharedPtr<LocationConfig>& location) { _locations.push_back(location); }
    bool hasLocations() const { return !_locations.empty(); }
    size_t getLocationsCount() const { return _locations.size(); }

 private:
    ServerConfig& operator=(const ServerConfig&);

    std::vector<Listen> _listens;
    std::vector<ServerName> _server_names;
    Return _return_value;
    std::vector<toolbox::SharedPtr<LocationConfig> > _locations;
    const HttpConfig* _parent;
};

}  // namespace config
