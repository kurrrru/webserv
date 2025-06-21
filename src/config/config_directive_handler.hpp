// Copyright 2025 Ideal Broccoli

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
    ~DirectiveParser();
    bool parseDirective(const std::vector<std::string>& tokens, size_t* pos, const std::string& directive, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool parseRootDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* root);
    bool parseIndexDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* index);
    bool parseAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, bool* autoindex);
    bool parseAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* allowedMethods);
    bool parseErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ErrorPage>* errorPages);
    bool parseUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* uploadStore);
    bool parseCgiPathDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgiPath);
    bool parseCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* cgiExtensions);
    bool parseReturnDirective(const std::vector<std::string>& tokens, size_t* pos, Return* returnValue);
    bool parseClientMaxBodySize(const std::vector<std::string>& tokens, size_t* pos, size_t* clientMaxBodySize);
    bool parseListenDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<Listen>* listen);
    bool parseServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<config::ServerName>* serverNames);
    bool isDirectiveAllowedInContext(const std::string& directive, DirectiveContext context) const;
    bool handleDuplicateDirective(const std::string& directiveName, const std::vector<std::string>& tokens, size_t* pos, bool* shouldSkip);

 private:
    DirectiveParser(const DirectiveParser& other);
    DirectiveParser& operator=(const DirectiveParser& other);

    std::map<std::string, DirectiveInfo> _directiveInfo;
    void initDirectiveInfo();
    bool handleAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleRootDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleIndexDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleCgiPathDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleReturnDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleClientMaxBodySizeDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleListenDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool handleServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location);
    bool isAllowedDuplicate(const std::string& directiveName);
    bool isIgnoredDuplicate(const std::string& directiveName);
    void skipUntilSemicolon(const std::vector<std::string>& tokens, size_t* pos);
};

}  // namespace config
