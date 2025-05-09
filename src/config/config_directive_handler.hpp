#pragma once

#include <string>
#include <map>
#include <vector>

#include "config.hpp"
#include "config_namespace.hpp"

namespace config {

enum DirectiveContext {
    CONTEXT_NONE = 0b000,
    CONTEXT_HTTP = 0b001,
    CONTEXT_SERVER = 0b010,
    CONTEXT_LOCATION = 0b100,
    CONTEXT_ALL = CONTEXT_HTTP | CONTEXT_SERVER | CONTEXT_LOCATION,
    CONTEXT_HTTP_SERVER = CONTEXT_HTTP | CONTEXT_SERVER,
    CONTEXT_HTTP_LOCATION = CONTEXT_HTTP | CONTEXT_LOCATION,
    CONTEXT_SERVER_LOCATION = CONTEXT_SERVER | CONTEXT_LOCATION,
};

struct DirectiveInfo {
    std::string directive;
    DirectiveContext context;

    DirectiveInfo()
        : directive(),
        context(CONTEXT_NONE) {}
};

class ServerConfig;
class LocationConfig;
class HttpConfig;
class DirectiveParser {
 public:
    DirectiveParser();
    bool parseDirective(const std::vector<std::string>& tokens, size_t* pos, const std::string& directive, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool parseRootDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* root);
    bool parseIndexDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* index);
    bool parseAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, bool* autoindex);
    bool parseAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* allowed_methods);
    bool parseErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ErrorPage>* error_pages);
    bool parseUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* upload_store);
    bool parseCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgi_pass);
    bool parseCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* cgi_extensions);
    bool parseReturnDirective(const std::vector<std::string>& tokens, size_t* pos, Return* return_value);
    bool parseClientMaxBodySize(const std::vector<std::string>& tokens, size_t* pos, size_t* client_max_body_size);
    bool parseListenDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<Listen>* listen);
    bool parseServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<config::ServerName>* server_names);
    bool isDirectiveAllowedInContext(const std::string& directive, DirectiveContext context) const;
    bool handleDuplicateDirective(const std::string& directive_name, const std::vector<std::string>& tokens, size_t* pos, bool* should_skip);

 private:
    std::map<std::string, DirectiveInfo> _directive_info;
    void initDirectiveInfo();
    bool handleAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleRootDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleIndexDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleReturnDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleClientMaxBodySizeDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleListenDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool isAllowedDuplicate(const std::string& directive_name);
    bool isIgnoredDuplicate(const std::string& directive_name);
    void skipUntilSemicolon(const std::vector<std::string>& tokens, size_t* pos);
};

}  // namespace config
