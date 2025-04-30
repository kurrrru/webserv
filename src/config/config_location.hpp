// Copyright 2025 Ideal Broccoli

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

class LocationConfig : public ConfigBase {
 public:
    LocationConfig();
    LocationConfig(const LocationConfig&);
    ~LocationConfig();

    void setParent(ServerConfig* config_server);
    void setParent(LocationConfig* config_location);
    ServerConfig* getServerParent() const;
    LocationConfig* getLocationParent() const;
    const std::vector<toolbox::SharedPtr<LocationConfig> >& getLocations() const;
    void addLocation(const LocationConfig& location);

    std::string path;
    Return return_value;
    std::vector<toolbox::SharedPtr<LocationConfig> > locations;

 private:
    ServerConfig* _parent_server;
    LocationConfig* _parent_location;
    LocationConfig& operator=(const LocationConfig&);
};

}  // namespace config
