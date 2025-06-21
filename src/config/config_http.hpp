#pragma once

#include <string>
#include <vector>

#include "config_namespace.hpp"
#include "config_base.hpp"
#include "config_server.hpp"
#include "config_location.hpp"

#include "../../toolbox/shared.hpp"

namespace config {
/**
 * @class HttpConfig
 * @brief Class for managing HTTP server configuration
 *
 * This class extends the ConfigBase class to provide HTTP-specific configuration
 * capabilities. It serves as the top-level configuration container that holds
 * multiple server configurations. Each server configuration represents a virtual
 * server that can listen on specific IP:port combinations.
 *
 * The HttpConfig class inherits all common configuration properties from ConfigBase
 * such as allowed methods, autoindex settings, clientMaxBodySize, etc., which
 * serve as default values for all contained servers.
 *
 * Usage example:
 * @code
 * HttpConfig httpConfig;
 * toolbox::SharedPtr<ServerConfig> server1(new ServerConfig());
 * toolbox::SharedPtr<ServerConfig> server2(new ServerConfig());
 * 
 * // Add servers to HTTP config
 * httpConfig.addServer(server1);
 * httpConfig.addServer(server2);
 * 
 * // Accessing the configured settings
 * const std::vector<toolbox::SharedPtr<ServerConfig> >& servers = httpConfig.getServers();
 * std::size_t serverCount = servers.size();  // Returns 2
 * @endcode
 */
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
