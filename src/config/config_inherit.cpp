// Copyright 2025 Ideal Broccoli

#include "config_inherit.hpp"
#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/shared.hpp"

namespace config {

ConfigInherit::ConfigInherit(HttpConfig* http_config) :
_http_config(http_config) {
}

ConfigInherit::~ConfigInherit() {
}

void ConfigInherit::applyInheritance() {
    // HttpConfigからServerConfigへの継承
    for (std::vector<toolbox::SharedPtr<ServerConfig> >::iterator server_it = _http_config->servers.begin();
        server_it != _http_config->servers.end(); ++server_it) {
        // 親HttpConfigから子ServerConfigへの継承
        config::ConfigInherit::inheritHttpToServer(_http_config, server_it->get());

        // ServerConfigからLocationConfigへの継承
        for (std::vector<toolbox::SharedPtr<LocationConfig> >::iterator loc_it = (*server_it)->locations.begin();
            loc_it != (*server_it)->locations.end(); ++loc_it) {
            // 親ServerConfigから子LocationConfigへの継承
            config::ConfigInherit::inheritServerToLocation(server_it->get(), loc_it->get());
            // さらにネストしたLocationがあれば再帰的に適用
            applyLocationInheritance(*loc_it->get());
        }
    }
}

void ConfigInherit::applyLocationInheritance(LocationConfig& location) {
    for (std::vector<toolbox::SharedPtr<LocationConfig> >::iterator child_it = location.locations.begin();
        child_it != location.locations.end(); ++child_it) {
        // 親Location→子Locationへの継承
        config::ConfigInherit::inheritLocationToLocation(&location, child_it->get());
        // さらにネストしたLocationがあれば再帰的に適用
        if (!(*child_it)->locations.empty()) {
            applyLocationInheritance(*child_it->get());
        }
    }
}

void ConfigInherit::inheritHttpToServer(HttpConfig* http, ServerConfig* server) {
    if (!http || !server) {
        return;
    }
    // allowed_methods
    if (server->allowed_methods.empty() && !http->allowed_methods.empty()) {
        server->allowed_methods = http->allowed_methods;
    }
    // autoindex
    if (server->autoindex == DEFAULT_AUTOINDEX &&
        http->autoindex != DEFAULT_AUTOINDEX) {
        server->autoindex = http->autoindex;
    }
    // cgi_extensions
    if (server->cgi_extensions.empty() && !http->cgi_extensions.empty()) {
        server->cgi_extensions = http->cgi_extensions;
    }
    // cgi_pass
    if (server->cgi_pass == DEFAULT_CGI_PATH &&
        http->cgi_pass != DEFAULT_CGI_PATH) {
        server->cgi_pass = http->cgi_pass;
    }
    // client_max_body_size
    if (server->client_max_body_size == DEFAULT_CLIENT_MAX_BODY_SIZE &&
        http->client_max_body_size != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        server->client_max_body_size = http->client_max_body_size;
    }
    // error_pages
    if (server->error_pages.empty() && !http->error_pages.empty()) {
        server->error_pages = http->error_pages;
    }
    // indices
    if (server->indices == DEFAULT_INDICES &&
        http->indices != DEFAULT_INDICES) {
        server->indices = http->indices;
    }
    // root
    if (server->root == DEFAULT_ROOT && http->root != DEFAULT_ROOT) {
        server->root = http->root;
    }
    // upload_store
    if (server->upload_store == DEFAULT_UPLOAD_STORE &&
        http->upload_store != DEFAULT_UPLOAD_STORE) {
        server->upload_store = http->upload_store;
    }
    // 親への参照を設定
    server->setParent(http);
}

void ConfigInherit::inheritServerToLocation(ServerConfig* server, LocationConfig* location) {
    if (!server || !location) {
        return;
    }
    // allowed_methods
    if (location->allowed_methods.empty() && !server->allowed_methods.empty()) {
        location->allowed_methods = server->allowed_methods;
    }
    // autoindex
    if (location->autoindex == DEFAULT_AUTOINDEX &&
        server->autoindex != DEFAULT_AUTOINDEX) {
        location->autoindex = server->autoindex;
    }
    // cgi_extensions
    if (location->cgi_extensions.empty() && !server->cgi_extensions.empty()) {
        location->cgi_extensions = server->cgi_extensions;
    }
    // cgi_pass
    if (location->cgi_pass == DEFAULT_CGI_PATH &&
        server->cgi_pass == DEFAULT_CGI_PATH) {
        location->cgi_pass = server->cgi_pass;
    }
    // client_max_body_size
    if (location->client_max_body_size == DEFAULT_CLIENT_MAX_BODY_SIZE && 
        server->client_max_body_size != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        location->client_max_body_size = server->client_max_body_size;
    }
    // error_pages
    if (location->error_pages.empty() && !server->error_pages.empty()) {
        location->error_pages = server->error_pages;
    }
    // indices
    if (location->indices == DEFAULT_INDICES &&
        server->indices != DEFAULT_INDICES) {
        location->indices = server->indices;
    }
    // root
    if (location->root == DEFAULT_ROOT && server->root != DEFAULT_ROOT) {
        location->root = server->root;
    }
    // upload_store
    if (location->upload_store == DEFAULT_UPLOAD_STORE &&
        server->upload_store != DEFAULT_UPLOAD_STORE) {
        location->upload_store = server->upload_store;
    }
    // 親への参照を設定
    location->setParent(server);
}

void ConfigInherit::inheritLocationToLocation(LocationConfig* parent, LocationConfig* child) {
    if (!parent || !child) {
        return;
    }
    // allowed_methods
    if (child->allowed_methods.empty() && !parent->allowed_methods.empty()) {
        child->allowed_methods = parent->allowed_methods;
    }
    // autoindex
    if (child->autoindex == DEFAULT_AUTOINDEX && 
        parent->autoindex != DEFAULT_AUTOINDEX) {
        child->autoindex = parent->autoindex;
    }
    // cgi_extensions
    if (child->cgi_extensions.empty() && !parent->cgi_extensions.empty()) {
        child->cgi_extensions = parent->cgi_extensions;
    }
    // cgi_pass
    if (child->cgi_pass == DEFAULT_CGI_PATH &&
        parent->cgi_pass == DEFAULT_CGI_PATH) {
        child->cgi_pass = parent->cgi_pass;
    }
    // client_max_body_size
    if (child->client_max_body_size == DEFAULT_CLIENT_MAX_BODY_SIZE && 
        parent->client_max_body_size != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        child->client_max_body_size = parent->client_max_body_size;
    }
    // error_pages
    if (child->error_pages.empty() && !parent->error_pages.empty()) {
        child->error_pages = parent->error_pages;
    }
    // indices
    if (child->indices == DEFAULT_INDICES &&
        parent->indices != DEFAULT_INDICES) {
        child->indices = parent->indices;
    }
    // root
    if (child->root == DEFAULT_ROOT && parent->root != DEFAULT_ROOT) {
        child->root = parent->root;
    }
    // upload_store
    if (child->upload_store == DEFAULT_UPLOAD_STORE &&
        parent->upload_store != DEFAULT_UPLOAD_STORE) {
        child->upload_store = parent->upload_store;
    }
    // 親への参照を設定
    child->setParent(parent);
}

}  // namespace config
