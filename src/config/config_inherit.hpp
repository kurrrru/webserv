#pragma once

#include "config_http.hpp"
#include "config_server.hpp"
#include "config_location.hpp"


namespace config {

class ConfigInherit {
 public:
    explicit ConfigInherit(HttpConfig* http_config);
    ~ConfigInherit();

    void applyInheritance();

 private:
    ConfigInherit();
    ConfigInherit(const ConfigInherit&);
    ConfigInherit& operator=(const ConfigInherit&);

    void applyLocationInheritance(LocationConfig& location);
    void inheritHttpToServer(const HttpConfig* http, ServerConfig* server);
    void inheritServerToLocation(const ServerConfig* server, LocationConfig* location);
    void inheritLocationToLocation(const LocationConfig* parent, LocationConfig* child);

    HttpConfig* _http_config;
};

}  // namespace config
