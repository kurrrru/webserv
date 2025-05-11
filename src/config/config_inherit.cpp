#include "config_inherit.hpp"
#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/shared.hpp"

namespace config {

ConfigInherit::ConfigInherit(HttpConfig* http_config) :
_http_config(http_config) {
}

ConfigInherit::~ConfigInherit() {
}

void indexEmptyCheck(HttpConfig* http) {
    if (http->getIndices().empty()) {
        http->setIndices(DEFAULT_INDICES);
    }
}

void serverEmptyCheck(ServerConfig* server) {
    if (server->getListens().empty()) {
        Listen listen;
        listen.setPort(DEFAULT_PORT);
        listen.setIp(DEFAULT_IP);
        listen.setDefaultServer(false);
        server->addListen(listen);
    }
    if (server->getServerNames().empty()) {
        ServerName server_name;
        server_name.setName(DEFAULT_SERVER_NAME);
        server_name.setType(config::ServerName::EXACT);
        server->addServerName(server_name);
    }
}

void ConfigInherit::applyInheritance() {
    indexEmptyCheck(_http_config);
    for (size_t i = 0; i < _http_config->getServers().size(); ++i) {
        ServerConfig* server = _http_config->getServers()[i].get();
        config::ConfigInherit::inheritHttpToServer(_http_config, server);
        serverEmptyCheck(server);
        for (size_t j = 0; j < server->getLocations().size(); j++) {
            LocationConfig* location = server->getLocations()[j].get();
            config::ConfigInherit::inheritServerToLocation(server, location);
            applyLocationInheritance(*location);
        }
    }
}

void ConfigInherit::applyLocationInheritance(LocationConfig& location) {
    for (size_t i = 0; i < location.getLocations().size(); ++i) {
        LocationConfig* child = location.getLocations()[i].get();
        config::ConfigInherit::inheritLocationToLocation(&location, child);
        if (!child->getLocations().empty()) {
            applyLocationInheritance(*child);
        }
    }
}

void ConfigInherit::inheritHttpToServer(const HttpConfig* http, ServerConfig* server) {
    if (!http || !server) {
        return;
    }
    if (server->getAllowedMethods().empty() && !http->getAllowedMethods().empty()) {
        server->setAllowedMethods(http->getAllowedMethods());
    }
    if (server->getAutoindex() == DEFAULT_AUTOINDEX &&
        http->getAutoindex() != DEFAULT_AUTOINDEX) {
        server->setAutoindex(http->getAutoindex());
    }
    if (server->getCgiExtensions().empty() && !http->getCgiExtensions().empty()) {
        server->setCgiExtensions(http->getCgiExtensions());
    }
    if (server->getCgiPass() == DEFAULT_CGI_PATH &&
        http->getCgiPass() != DEFAULT_CGI_PATH) {
        server->setCgiPass(http->getCgiPass());
    }
    if (server->getClientMaxBodySize() == DEFAULT_CLIENT_MAX_BODY_SIZE &&
        http->getClientMaxBodySize() != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        server->setClientMaxBodySize(http->getClientMaxBodySize());
    }
    if (server->getErrorPages().empty() && !http->getErrorPages().empty()) {
        for (size_t i = 0; i < http->getErrorPages().size(); ++i) {
            server->addErrorPage(http->getErrorPages()[i]);
        }
    }
    if (server->getIndices().empty() && !http->getIndices().empty()) {
        server->setIndices(http->getIndices());
    }
    if (server->getRoot() == DEFAULT_ROOT && http->getRoot() != DEFAULT_ROOT) {
        server->setRoot(http->getRoot());
    }
    if (server->getUploadStore() == DEFAULT_UPLOAD_STORE &&
        http->getUploadStore() != DEFAULT_UPLOAD_STORE) {
        server->setUploadStore(http->getUploadStore());
    }
    server->setHttpParent(http);
}

void ConfigInherit::inheritServerToLocation(const ServerConfig* server, LocationConfig* location) {
    if (!server || !location) {
        return;
    }
    if (location->getAllowedMethods().empty() && !server->getAllowedMethods().empty()) {
        location->setAllowedMethods(server->getAllowedMethods());
    }
    if (location->getAutoindex() == DEFAULT_AUTOINDEX &&
        server->getAutoindex() != DEFAULT_AUTOINDEX) {
        location->setAutoindex(server->getAutoindex());
    }
    if (location->getCgiExtensions().empty() && !server->getCgiExtensions().empty()) {
        location->setCgiExtensions(server->getCgiExtensions());
    }
    if (location->getCgiPass() == DEFAULT_CGI_PATH &&
        server->getCgiPass() != DEFAULT_CGI_PATH) {
        location->setCgiPass(server->getCgiPass());
    }
    if (location->getClientMaxBodySize() == DEFAULT_CLIENT_MAX_BODY_SIZE &&
        server->getClientMaxBodySize() != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        location->setClientMaxBodySize(server->getClientMaxBodySize());
    }
    if (location->getErrorPages().empty() && !server->getErrorPages().empty()) {
        for (size_t i = 0; i < server->getErrorPages().size(); ++i) {
            location->addErrorPage(server->getErrorPages()[i]);
        }
    }
    if (location->getIndices().empty() && !server->getIndices().empty()) {
        location->setIndices(server->getIndices());
    }
    if (location->getRoot() == DEFAULT_ROOT && server->getRoot() != DEFAULT_ROOT) {
        location->setRoot(server->getRoot());
    }
    if (location->getUploadStore() == DEFAULT_UPLOAD_STORE &&
        server->getUploadStore() != DEFAULT_UPLOAD_STORE) {
        location->setUploadStore(server->getUploadStore());
    }
    location->setServerParent(server);
}

void ConfigInherit::inheritLocationToLocation(const LocationConfig* parent, LocationConfig* child) {
    if (!parent || !child) {
        return;
    }
    if (child->getAllowedMethods().empty() && !parent->getAllowedMethods().empty()) {
        child->setAllowedMethods(parent->getAllowedMethods());
    }
    if (child->getAutoindex() == DEFAULT_AUTOINDEX &&
        parent->getAutoindex() != DEFAULT_AUTOINDEX) {
        child->setAutoindex(parent->getAutoindex());
    }
    if (child->getCgiExtensions().empty() && !parent->getCgiExtensions().empty()) {
        child->setCgiExtensions(parent->getCgiExtensions());
    }
    if (child->getCgiPass() == DEFAULT_CGI_PATH &&
        parent->getCgiPass() != DEFAULT_CGI_PATH) {
        child->setCgiPass(parent->getCgiPass());
    }
    if (child->getClientMaxBodySize() == DEFAULT_CLIENT_MAX_BODY_SIZE &&
        parent->getClientMaxBodySize() != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        child->setClientMaxBodySize(parent->getClientMaxBodySize());
    }
    if (child->getErrorPages().empty() && !parent->getErrorPages().empty()) {
        for (size_t i = 0; i < parent->getErrorPages().size(); ++i) {
            child->addErrorPage(parent->getErrorPages()[i]);
        }
    }
    if (child->getIndices().empty() && !parent->getIndices().empty()) {
        child->setIndices(parent->getIndices());
    }
    if (child->getRoot() == DEFAULT_ROOT && parent->getRoot() != DEFAULT_ROOT) {
        child->setRoot(parent->getRoot());
    }
    if (child->getUploadStore() == DEFAULT_UPLOAD_STORE &&
        parent->getUploadStore() != DEFAULT_UPLOAD_STORE) {
        child->setUploadStore(parent->getUploadStore());
    }
    child->setLocationParent(parent);
}

}  // namespace config