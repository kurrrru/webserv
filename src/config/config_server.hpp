// Copyright 2025 Ideal Broccoli

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

    void setParent(HttpConfig* config_http);
    HttpConfig* getParent() const;
    void addLocation(const LocationConfig& location);
    const std::vector<toolbox::SharedPtr<LocationConfig> >& getLocations() const;

    std::vector<Listen> listens;
    std::vector<ServerName> server_names;
    Return return_value;
    std::vector<toolbox::SharedPtr<LocationConfig> > locations;

 private:
    HttpConfig* _parent;
    ServerConfig& operator=(const ServerConfig&);
};

}  // namespace config
