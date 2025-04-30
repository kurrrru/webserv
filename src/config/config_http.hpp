// Copyright 2025 Ideal Broccoli

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

    void addServer(const ServerConfig& server);
    const std::vector<toolbox::SharedPtr<ServerConfig> >& getServers() const;

    std::vector<toolbox::SharedPtr<ServerConfig> > servers;
 private:
    HttpConfig& operator=(const HttpConfig&);
};

}  // namespace config
