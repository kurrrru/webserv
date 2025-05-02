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
    for (std::vector<toolbox::SharedPtr<ServerConfig> >::iterator server_it = _http_config->servers.begin();
        server_it != _http_config->servers.end(); ++server_it) {
        config::ConfigInherit::inheritHttpToServer(_http_config, server_it->get());
        for (std::vector<toolbox::SharedPtr<LocationConfig> >::iterator loc_it = (*server_it)->locations.begin();
            loc_it != (*server_it)->locations.end(); ++loc_it) {
            config::ConfigInherit::inheritServerToLocation(server_it->get(), loc_it->get());
            applyLocationInheritance(*loc_it->get());
        }
    }
}

void ConfigInherit::applyLocationInheritance(LocationConfig& location) {
    for (std::vector<toolbox::SharedPtr<LocationConfig> >::iterator child_it = location.locations.begin();
        child_it != location.locations.end(); ++child_it) {
        config::ConfigInherit::inheritLocationToLocation(&location, child_it->get());
        if (!(*child_it)->locations.empty()) {
            applyLocationInheritance(*child_it->get());
        }
    }
}

void ConfigInherit::inheritHttpToServer(HttpConfig* http, ServerConfig* server) {
    if (!http || !server) {
        return;
    }
    if (server->allowed_methods.empty() && !http->allowed_methods.empty()) {
        server->allowed_methods = http->allowed_methods;
    }
    if (server->autoindex == DEFAULT_AUTOINDEX &&
        http->autoindex != DEFAULT_AUTOINDEX) {
        server->autoindex = http->autoindex;
    }
    if (server->cgi_extensions.empty() && !http->cgi_extensions.empty()) {
        server->cgi_extensions = http->cgi_extensions;
    }
    if (server->cgi_pass == DEFAULT_CGI_PATH &&
        http->cgi_pass != DEFAULT_CGI_PATH) {
        server->cgi_pass = http->cgi_pass;
    }
    if (server->client_max_body_size == DEFAULT_CLIENT_MAX_BODY_SIZE &&
        http->client_max_body_size != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        server->client_max_body_size = http->client_max_body_size;
    }
    if (server->error_pages.empty() && !http->error_pages.empty()) {
        server->error_pages = http->error_pages;
    }
    if (http->indices.empty()) {
        http->indices = DEFAULT_INDICES;
    }
    if (server->indices.empty() && !http->indices.empty()) {
        server->indices = http->indices;
    }
    if (server->root == DEFAULT_ROOT && http->root != DEFAULT_ROOT) {
        server->root = http->root;
    }
    if (server->upload_store == DEFAULT_UPLOAD_STORE &&
        http->upload_store != DEFAULT_UPLOAD_STORE) {
        server->upload_store = http->upload_store;
    }
    server->setParent(http);
}

void ConfigInherit::inheritServerToLocation(ServerConfig* server, LocationConfig* location) {
    if (!server || !location) {
        return;
    }
    if (location->allowed_methods.empty() && !server->allowed_methods.empty()) {
        location->allowed_methods = server->allowed_methods;
    }
    if (location->autoindex == DEFAULT_AUTOINDEX &&
        server->autoindex != DEFAULT_AUTOINDEX) {
        location->autoindex = server->autoindex;
    }
    if (location->cgi_extensions.empty() && !server->cgi_extensions.empty()) {
        location->cgi_extensions = server->cgi_extensions;
    }
    if (location->cgi_pass == DEFAULT_CGI_PATH &&
        server->cgi_pass == DEFAULT_CGI_PATH) {
        location->cgi_pass = server->cgi_pass;
    }
    if (location->client_max_body_size == DEFAULT_CLIENT_MAX_BODY_SIZE &&
        server->client_max_body_size != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        location->client_max_body_size = server->client_max_body_size;
    }
    if (location->error_pages.empty() && !server->error_pages.empty()) {
        location->error_pages = server->error_pages;
    }
    if (location->indices.empty() && !server->indices.empty()) {
        location->indices = server->indices;
    }
    if (location->root == DEFAULT_ROOT && server->root != DEFAULT_ROOT) {
        location->root = server->root;
    }
    if (location->upload_store == DEFAULT_UPLOAD_STORE &&
        server->upload_store != DEFAULT_UPLOAD_STORE) {
        location->upload_store = server->upload_store;
    }
    location->setParent(server);
}

void ConfigInherit::inheritLocationToLocation(LocationConfig* parent, LocationConfig* child) {
    if (!parent || !child) {
        return;
    }
    if (child->allowed_methods.empty() && !parent->allowed_methods.empty()) {
        child->allowed_methods = parent->allowed_methods;
    }
    if (child->autoindex == DEFAULT_AUTOINDEX &&
        parent->autoindex != DEFAULT_AUTOINDEX) {
        child->autoindex = parent->autoindex;
    }
    if (child->cgi_extensions.empty() && !parent->cgi_extensions.empty()) {
        child->cgi_extensions = parent->cgi_extensions;
    }
    if (child->cgi_pass == DEFAULT_CGI_PATH &&
        parent->cgi_pass == DEFAULT_CGI_PATH) {
        child->cgi_pass = parent->cgi_pass;
    }
    if (child->client_max_body_size == DEFAULT_CLIENT_MAX_BODY_SIZE &&
        parent->client_max_body_size != DEFAULT_CLIENT_MAX_BODY_SIZE) {
        child->client_max_body_size = parent->client_max_body_size;
    }
    if (child->error_pages.empty() && !parent->error_pages.empty()) {
        child->error_pages = parent->error_pages;
    }
    if (child->indices.empty() && !parent->indices.empty()) {
        child->indices = parent->indices;
    }
    if (child->root == DEFAULT_ROOT && parent->root != DEFAULT_ROOT) {
        child->root = parent->root;
    }
    if (child->upload_store == DEFAULT_UPLOAD_STORE &&
        parent->upload_store != DEFAULT_UPLOAD_STORE) {
        child->upload_store = parent->upload_store;
    }
    child->setParent(parent);
}

}  // namespace config
