#include <map>
#include <string>
#include <vector>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"
#include "config_util.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

DirectiveParser::DirectiveParser() {
    initDirectiveInfo();
}

void DirectiveParser::initDirectiveInfo() {
    DirectiveInfo info;

    info.directive = config::directive::ROOT;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::ROOT] = info;

    info.directive = config::directive::INDEX;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::INDEX] = info;

    info.directive = config::directive::AUTOINDEX;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::AUTOINDEX] = info;

    info.directive = config::directive::ALLOWED_METHODS;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::ALLOWED_METHODS] = info;

    info.directive = config::directive::ERROR_PAGE;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::ERROR_PAGE] = info;

    info.directive = config::directive::UPLOAD_STORE;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::UPLOAD_STORE] = info;

    info.directive = config::directive::CGI_PATH;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::CGI_PATH] = info;

    info.directive = config::directive::CGI_EXTENSION;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::CGI_EXTENSION] = info;

    info.directive = config::directive::CLIENT_MAX_BODY_SIZE;
    info.context = CONTEXT_ALL;
    _directiveInfo[config::directive::CLIENT_MAX_BODY_SIZE] = info;

    info.directive = config::directive::LISTEN;
    info.context = CONTEXT_SERVER;
    _directiveInfo[config::directive::LISTEN] = info;

    info.directive = config::directive::SERVER_NAME;
    info.context = CONTEXT_SERVER;
    _directiveInfo[config::directive::SERVER_NAME] = info;

    info.directive = config::directive::RETURN;
    info.context = CONTEXT_SERVER_LOCATION;
    _directiveInfo[config::directive::RETURN] = info;
}

bool DirectiveParser::parseDirective(const std::vector<std::string>& tokens, size_t* pos, const std::string& directive, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (directive == config::directive::ROOT) {
        return handleRootDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::INDEX) {
        return handleIndexDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::AUTOINDEX) {
        return handleAutoindexDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::ALLOWED_METHODS) {
        return handleAllowedMethodsDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::ERROR_PAGE) {
        return handleErrorPageDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::UPLOAD_STORE) {
        return handleUploadStoreDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::CGI_PATH) {
        return handleCgiPathDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::CGI_EXTENSION) {
        return handleCgiExtensionDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::RETURN) {
        return handleReturnDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::CLIENT_MAX_BODY_SIZE) {
        return handleClientMaxBodySizeDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::LISTEN) {
        return handleListenDirective(tokens, pos, http, server, location);
    } else if (directive == config::directive::SERVER_NAME) {
        return handleServerNameDirective(tokens, pos, http, server, location);
    }
    return false;
}

bool DirectiveParser::isDirectiveAllowedInContext(const std::string& directive, DirectiveContext context) const {
    std::map<std::string, DirectiveInfo>::const_iterator it = _directiveInfo.find(directive);
    if (it == _directiveInfo.end()) {
        return false;
    }
    if (!((it->second.context & context) != 0)) {
        throwConfigError("directive \"" + directive + "\" is not allowed here");
    }
    return true;
}

bool DirectiveParser::handleAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::vector<std::string> methods;
    if (http) {
        methods = http->getAllowedMethods();
    } else if (server) {
        methods = server->getAllowedMethods();
    } else if (location) {
        methods = location->getAllowedMethods();
    }
    bool result = parseAllowedMethodsDirective(tokens, pos, &methods);
    if (result) {
        if (http) {
            http->setAllowedMethods(methods);
        } else if (server) {
            server->setAllowedMethods(methods);
        } else if (location) {
            location->setAllowedMethods(methods);
        }
    }
    return result;
}

bool DirectiveParser::handleAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    bool autoindex;
    if (http) {
        autoindex = http->getAutoindex();
    } else if (server) {
        autoindex = server->getAutoindex();
    } else if (location) {
        autoindex = location->getAutoindex();
    } else {
        return false;
    }
    bool result = parseAutoindexDirective(tokens, pos, &autoindex);
    if (result) {
        if (http) {
            http->setAutoindex(autoindex);
        } else if (server) {
            server->setAutoindex(autoindex);
        } else if (location) {
            location->setAutoindex(autoindex);
        }
    }
    return result;
}

bool DirectiveParser::handleCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::vector<std::string> extensions;
    if (http) {
        extensions = http->getCgiExtensions();
    } else if (server) {
        extensions = server->getCgiExtensions();
    } else if (location) {
        extensions = location->getCgiExtensions();
    } else {
        return false;
    }
    bool result = parseCgiExtensionDirective(tokens, pos, &extensions);
    if (result) {
        if (http) {
            http->setCgiExtensions(extensions);
        } else if (server) {
            server->setCgiExtensions(extensions);
        } else if (location) {
            location->setCgiExtensions(extensions);
        }
    }
    return result;
}

bool DirectiveParser::handleCgiPathDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::string cgiPath;
    if (http) {
        cgiPath = http->getCgiPath();
    } else if (server) {
        cgiPath = server->getCgiPath();
    } else if (location) {
        cgiPath = location->getCgiPath();
    } else {
        return false;
    }
    bool result = parseCgiPathDirective(tokens, pos, &cgiPath);
    if (result) {
        if (http) {
            http->setCgiPath(cgiPath);
        } else if (server) {
            server->setCgiPath(cgiPath);
        } else if (location) {
            location->setCgiPath(cgiPath);
        }
    }
    return result;
}

bool DirectiveParser::handleErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::vector<ErrorPage> errorPages;
    if (http) {
        errorPages = http->getErrorPages();
    } else if (server) {
        errorPages = server->getErrorPages();
    } else if (location) {
        errorPages = location->getErrorPages();
    } else {
        return false;
    }
    bool result = parseErrorPageDirective(tokens, pos, &errorPages);
    if (result) {
        if (http) {
            for (size_t i = 0; i < errorPages.size(); ++i) {
                http->addErrorPage(errorPages[i]);
            }
        } else if (server) {
            for (size_t i = 0; i < errorPages.size(); ++i) {
                server->addErrorPage(errorPages[i]);
            }
        } else if (location) {
            for (size_t i = 0; i < errorPages.size(); ++i) {
                location->addErrorPage(errorPages[i]);
            }
        }
    }
    return result;
}

bool DirectiveParser::handleClientMaxBodySizeDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    size_t size;
    if (http) {
        size = http->getClientMaxBodySize();
    } else if (server) {
        size = server->getClientMaxBodySize();
    } else if (location) {
        size = location->getClientMaxBodySize();
    } else {
        return false;
    }
    bool result = parseClientMaxBodySize(tokens, pos, &size);
    if (result) {
        if (http) {
            http->setClientMaxBodySize(size);
        } else if (server) {
            server->setClientMaxBodySize(size);
        } else if (location) {
            location->setClientMaxBodySize(size);
        }
    }
    return result;
}

bool DirectiveParser::handleIndexDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::vector<std::string> indices;
    if (http) {
        indices = http->getIndices();
    } else if (server) {
        indices = server->getIndices();
    } else if (location) {
        indices = location->getIndices();
    } else {
        return false;
    }
    bool result = parseIndexDirective(tokens, pos, &indices);
    if (result) {
        if (http) {
            http->setIndices(indices);
        } else if (server) {
            server->setIndices(indices);
        } else if (location) {
            location->setIndices(indices);
        }
    }
    return result;
}

bool DirectiveParser::handleListenDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        throwConfigError("\"" + std::string(config::directive::LISTEN) + "\" directive is not allowed here");
    } else if (location) {
        throwConfigError("\"" + std::string(config::directive::LISTEN) + "\" directive is not allowed here");
    }
    if (!server) {
        return false;
    }
    std::vector<Listen> listens = server->getListens();
    bool result = parseListenDirective(tokens, pos, &listens);
    if (result) {
        server->setListens(listens);
    }
    return result;
}

bool DirectiveParser::handleReturnDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        throwConfigError("\"" + std::string(config::directive::RETURN) + "\" directive is not allowed here");
    }
    Return returnValue;
    if (server) {
        returnValue = server->getReturnValue();
    } else if (location) {
        returnValue = location->getReturnValue();
    } else {
        return false;
    }
    bool result = parseReturnDirective(tokens, pos, &returnValue);
    if (result) {
        if (server) {
            server->setReturnValue(returnValue);
        } else if (location) {
            location->setReturnValue(returnValue);
        }
    }
    return result;
}

bool DirectiveParser::handleRootDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::string root;
    if (http) {
        root = http->getRoot();
    } else if (server) {
        root = server->getRoot();
    } else if (location) {
        root = location->getRoot();
    } else {
        return false;
    }
    bool result = parseRootDirective(tokens, pos, &root);
    if (result) {
        if (http) {
            http->setRoot(root);
        } else if (server) {
            server->setRoot(root);
        } else if (location) {
            location->setRoot(root);
        }
    }
    return result;
}

bool DirectiveParser::handleServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        throwConfigError("\"" + std::string(config::directive::SERVER_NAME) + "\" directive is not allowed here");
    } else if (location) {
        throwConfigError("\"" + std::string(config::directive::SERVER_NAME) + "\" directive is not allowed here");
    }
    if (!server) {
        return false;
    }
    std::vector<ServerName> serverNames = server->getServerNames();
    bool result = parseServerNameDirective(tokens, pos, &serverNames);
    if (result) {
        server->setServerNames(serverNames);
    }
    return result;
}

bool DirectiveParser::handleUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::string uploadStore;
    if (http) {
        uploadStore = http->getUploadStore();
    } else if (server) {
        uploadStore = server->getUploadStore();
    } else if (location) {
        uploadStore = location->getUploadStore();
    } else {
        return false;
    }
    bool result = parseUploadStoreDirective(tokens, pos, &uploadStore);
    if (result) {
        if (http) {
            http->setUploadStore(uploadStore);
        } else if (server) {
            server->setUploadStore(uploadStore);
        } else if (location) {
            location->setUploadStore(uploadStore);
        }
    }
    return result;
}

bool DirectiveParser::handleDuplicateDirective(const std::string& directiveName, const std::vector<std::string>& tokens, size_t* pos, bool* shouldSkip) {
    *shouldSkip = false;
    if (isAllowedDuplicate(directiveName)) {
        return true;
    } else if (isIgnoredDuplicate(directiveName)) {
        skipUntilSemicolon(tokens, pos);
        *shouldSkip = true;
        return true;
    } else {
        throwConfigError("\"" + directiveName + "\" directive is duplicate");
    }
    return false;
}

bool DirectiveParser::isAllowedDuplicate(const std::string& directiveName) {
    const std::string allowedDuplicates[] = {
        config::directive::ALLOWED_METHODS,
        config::directive::CGI_EXTENSION,
        config::directive::ERROR_PAGE,
        config::directive::INDEX,
        config::directive::SERVER_NAME,
        config::directive::LISTEN,
    };
    const size_t allowedCount = sizeof(allowedDuplicates) / sizeof(allowedDuplicates[0]);
    for (size_t i = 0; i < allowedCount; ++i) {
        if (directiveName == allowedDuplicates[i]) {
            return true;
        }
    }
    return false;
}

bool DirectiveParser::isIgnoredDuplicate(const std::string& directiveName) {
    const std::string ignoredDuplicates[] = {
        config::directive::RETURN
    };
    const size_t ignoredCount = sizeof(ignoredDuplicates) / sizeof(ignoredDuplicates[0]);
    for (size_t i = 0; i < ignoredCount; ++i) {
        if (directiveName == ignoredDuplicates[i]) {
            return true;
        }
    }
    return false;
}

void DirectiveParser::skipUntilSemicolon(const std::vector<std::string>& tokens, size_t* pos) {
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        (*pos)++;
    }
    if (*pos < tokens.size()) {
        (*pos)++;
    }
}
}  // namespace config
