#pragma once

#include <string>
#include <vector>

#include "config_namespace.hpp"
#include "config_base.hpp"
#include "config_server.hpp"
#include "config_location.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

class HttpConfig : public ConfigBase {
 public:
    HttpConfig();
    HttpConfig(const HttpConfig&);
    virtual ~HttpConfig();

    const std::vector<toolbox::SharedPtr<ServerConfig> >& getServers() const { return _servers; }
    void addServer(const toolbox::SharedPtr<ServerConfig>& server) { _servers.push_back(server); }

 private:
    std::vector<toolbox::SharedPtr<ServerConfig> > _servers;
    HttpConfig& operator=(const HttpConfig&);
};

}  // namespace config
