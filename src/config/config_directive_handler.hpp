// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <map>
#include <vector>

#include "config.hpp"
#include "config_namespace.hpp"

namespace config {
enum DirectiveContext {
    CONTEXT_NONE = 0,
    CONTEXT_HTTP = 1,
    CONTEXT_SERVER = 2,
    CONTEXT_LOCATION = 4,
    CONTEXT_ALL = CONTEXT_HTTP | CONTEXT_SERVER | CONTEXT_LOCATION,
    CONTEXT_HTTP_SERVER = CONTEXT_HTTP | CONTEXT_SERVER,
    CONTEXT_HTTP_LOCATION = CONTEXT_HTTP | CONTEXT_LOCATION,
    CONTEXT_SERVER_LOCATION = CONTEXT_SERVER | CONTEXT_LOCATION,
};
struct DirectiveInfo {
    std::string directive;
    DirectiveContext context;

    DirectiveInfo(const std::string& n = "", DirectiveContext c = CONTEXT_NONE) : directive(n), context(c) {}
};
class DirectiveParser {
 public:
    DirectiveParser();
    bool parseDirective(const std::vector<std::string>& tokens, size_t* pos, const std::string& directive, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* config);
    bool parseRootDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* root);
    bool parseIndexDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* index);
    bool parseAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, bool* autoindex);
    bool parseAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* allowed_methods);
    bool parseErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ErrorPage>* error_pages);
    bool parseUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* upload_store);
    bool parseCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgi_pass);
    bool parseCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgi_extension);
    bool parseReturnDirective(const std::vector<std::string>& tokens, size_t* pos, int* return_code, std::string* return_url);
    bool parseClientMaxBodySize(const std::vector<std::string>& tokens, size_t* pos, size_t* client_max_body_size);
    bool parseListenDirective(const std::vector<std::string>& tokens, size_t* pos, int* port);
    bool parseServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* server_name);
    bool isDirectiveAllowedInContext(const std::string& directive, DirectiveContext context) const;

 private:
    std::map<std::string, DirectiveInfo> _directive_info;
    void initDirectiveInfo();
    DirectiveContext getCurrentContext(config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* config) const;
    bool handleAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleRootDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleIndexDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleReturnDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleClientMaxBodySizeDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleListenDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
    bool handleServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http);
};

}  // namespace config
