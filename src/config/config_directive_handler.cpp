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
    _directive_info[config::directive::ROOT] = info;

    info.directive = config::directive::INDEX;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::INDEX] = info;

    info.directive = config::directive::AUTOINDEX;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::AUTOINDEX] = info;

    info.directive = config::directive::ALLOWED_METHODS;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::ALLOWED_METHODS] = info;

    info.directive = config::directive::ERROR_PAGE;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::ERROR_PAGE] = info;

    info.directive = config::directive::UPLOAD_STORE;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::UPLOAD_STORE] = info;

    info.directive = config::directive::CGI_PASS;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::CGI_PASS] = info;

    info.directive = config::directive::CGI_EXTENSION;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::CGI_EXTENSION] = info;

    info.directive = config::directive::CLIENT_MAX_BODY_SIZE;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::CLIENT_MAX_BODY_SIZE] = info;

    info.directive = config::directive::LISTEN;
    info.context = CONTEXT_SERVER;
    _directive_info[config::directive::LISTEN] = info;

    info.directive = config::directive::SERVER_NAME;
    info.context = CONTEXT_SERVER;
    _directive_info[config::directive::SERVER_NAME] = info;

    info.directive = config::directive::RETURN;
    info.context = CONTEXT_SERVER_LOCATION;
    _directive_info[config::directive::RETURN] = info;
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
    } else if (directive == config::directive::CGI_PASS) {
        return handleCgiPassDirective(tokens, pos, http, server, location);
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
    std::map<std::string, DirectiveInfo>::const_iterator it = _directive_info.find(directive);
    if (it == _directive_info.end()) {
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

bool DirectiveParser::handleCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::string cgi_pass;
    if (http) {
        cgi_pass = http->getCgiPass();
    } else if (server) {
        cgi_pass = server->getCgiPass();
    } else if (location) {
        cgi_pass = location->getCgiPass();
    } else {
        return false;
    }
    bool result = parseCgiPassDirective(tokens, pos, &cgi_pass);
    if (result) {
        if (http) {
            http->setCgiPass(cgi_pass);
        } else if (server) {
            server->setCgiPass(cgi_pass);
        } else if (location) {
            location->setCgiPass(cgi_pass);
        }
    }
    return result;
}

bool DirectiveParser::handleErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::vector<ErrorPage> error_pages;
    if (http) {
        error_pages = http->getErrorPages();
    } else if (server) {
        error_pages = server->getErrorPages();
    } else if (location) {
        error_pages = location->getErrorPages();
    } else {
        return false;
    }
    bool result = parseErrorPageDirective(tokens, pos, &error_pages);
    if (result) {
        if (http) {
            for (size_t i = 0; i < error_pages.size(); ++i) {
                http->addErrorPage(error_pages[i]);
            }
        } else if (server) {
            for (size_t i = 0; i < error_pages.size(); ++i) {
                server->addErrorPage(error_pages[i]);
            }
        } else if (location) {
            for (size_t i = 0; i < error_pages.size(); ++i) {
                location->addErrorPage(error_pages[i]);
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
    Return return_value;
    if (server) {
        return_value = server->getReturnValue();
    } else if (location) {
        return_value = location->getReturnValue();
    } else {
        return false;
    }
    bool result = parseReturnDirective(tokens, pos, &return_value);
    if (result) {
        if (server) {
            server->setReturnValue(return_value);
        } else if (location) {
            location->setReturnValue(return_value);
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
    std::vector<ServerName> server_names = server->getServerNames();
    bool result = parseServerNameDirective(tokens, pos, &server_names);
    if (result) {
        server->setServerNames(server_names);
    }
    return result;
}

bool DirectiveParser::handleUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    std::string upload_store;
    if (http) {
        upload_store = http->getUploadStore();
    } else if (server) {
        upload_store = server->getUploadStore();
    } else if (location) {
        upload_store = location->getUploadStore();
    } else {
        return false;
    }
    bool result = parseUploadStoreDirective(tokens, pos, &upload_store);
    if (result) {
        if (http) {
            http->setUploadStore(upload_store);
        } else if (server) {
            server->setUploadStore(upload_store);
        } else if (location) {
            location->setUploadStore(upload_store);
        }
    }
    return result;
}

bool DirectiveParser::handleDuplicateDirective(const std::string& directive_name, const std::vector<std::string>& tokens, size_t* pos, bool* should_skip) {
    *should_skip = false;
    if (isAllowedDuplicate(directive_name)) {
        return true;
    } else if (isIgnoredDuplicate(directive_name)) {
        skipUntilSemicolon(tokens, pos);
        *should_skip = true;
        return true;
    } else {
        throwConfigError("\"" + directive_name + "\" directive is duplicate");
    }
    return false;
}

bool DirectiveParser::isAllowedDuplicate(const std::string& directive_name) {
    const std::string allowed_duplicates[] = {
        config::directive::ALLOWED_METHODS,
        config::directive::CGI_EXTENSION,
        config::directive::ERROR_PAGE,
        config::directive::INDEX,
        config::directive::SERVER_NAME,
        config::directive::LISTEN,
    };
    const size_t allowed_count = sizeof(allowed_duplicates) / sizeof(allowed_duplicates[0]);
    for (size_t i = 0; i < allowed_count; ++i) {
        if (directive_name == allowed_duplicates[i]) {
            return true;
        }
    }
    return false;
}

bool DirectiveParser::isIgnoredDuplicate(const std::string& directive_name) {
    const std::string ignored_duplicates[] = {
        config::directive::RETURN
    };
    const size_t ignored_count = sizeof(ignored_duplicates) / sizeof(ignored_duplicates[0]);
    for (size_t i = 0; i < ignored_count; ++i) {
        if (directive_name == ignored_duplicates[i]) {
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
