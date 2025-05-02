// Copyright 2025 Ideal Broccoli

#include <map>
#include <string>
#include <vector>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

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
    } else {
        toolbox::logger::StepMark::error("Unknown directive \"" + directive + "\"");
        return false;
    }
    toolbox::logger::StepMark::error("Internal Error: Directive '" + directive + "' was allowed but no handler was executed.");
    return false;
}

bool DirectiveParser::isDirectiveAllowedInContext(const std::string& directive, DirectiveContext context) const {
    std::map<std::string, DirectiveInfo>::const_iterator it = _directive_info.find(directive);
    if (it == _directive_info.end()) {
        return false;
    }
    return (it->second.context & context) != 0;
}

bool DirectiveParser::handleAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseAllowedMethodsDirective(tokens, pos, &http->allowed_methods);
    } else if (server) {
        return parseAllowedMethodsDirective(tokens, pos, &server->allowed_methods);
    } else if (location) {
        return parseAllowedMethodsDirective(tokens, pos, &location->allowed_methods);
    }
    return false;
}

bool DirectiveParser::handleAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseAutoindexDirective(tokens, pos, &http->autoindex);
    } else if (server) {
        return parseAutoindexDirective(tokens, pos, &server->autoindex);
    } else if (location) {
        return parseAutoindexDirective(tokens, pos, &location->autoindex);
    }
    return false;
}

bool DirectiveParser::handleRootDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseRootDirective(tokens, pos, &http->root);
    } else if (location) {
        return parseRootDirective(tokens, pos, &location->root);
    } else if (server) {
        return parseRootDirective(tokens, pos, &server->root);
    }
    return false;
}

bool DirectiveParser::handleIndexDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseIndexDirective(tokens, pos, &http->indices);
    } else if (server) {
        return parseIndexDirective(tokens, pos, &server->indices);
    } else if (location) {
        return parseIndexDirective(tokens, pos, &location->indices);
    }
    return false;
}

bool DirectiveParser::handleErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseErrorPageDirective(tokens, pos, &http->error_pages);
    } else if (server) {
        return parseErrorPageDirective(tokens, pos, &server->error_pages);
    } else if (location) {
        return parseErrorPageDirective(tokens, pos, &location->error_pages);
    }
    return false;
}

bool DirectiveParser::handleUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseUploadStoreDirective(tokens, pos, &http->upload_store);
    } else if (server) {
        return parseUploadStoreDirective(tokens, pos, &server->upload_store);
    } else if (location) {
        return parseUploadStoreDirective(tokens, pos, &location->upload_store);
    }
    return false;
}

bool DirectiveParser::handleCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseCgiPassDirective(tokens, pos, &http->cgi_pass);
    } else if (server) {
        return parseCgiPassDirective(tokens, pos, &server->cgi_pass);
    } else if (location) {
        return parseCgiPassDirective(tokens, pos, &location->cgi_pass);
    }
    return false;
}

bool DirectiveParser::handleCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseCgiExtensionDirective(tokens, pos, &http->cgi_extensions);
    } else if (server) {
        return parseCgiExtensionDirective(tokens, pos, &server->cgi_extensions);
    } else if (location) {
        return parseCgiExtensionDirective(tokens, pos, &location->cgi_extensions);
    }
    return false;
}

bool DirectiveParser::handleReturnDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        toolbox::logger::StepMark::error("\"" + std::string(config::directive::RETURN) + "\" directive is not allowed here");
    } else if (server) {
        return parseReturnDirective(tokens, pos, &server->return_value);
    } else if (location) {
        return parseReturnDirective(tokens, pos, &location->return_value);
    }
    return false;
}

bool DirectiveParser::handleClientMaxBodySizeDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        return parseClientMaxBodySize(tokens, pos, &http->client_max_body_size);
    } else if (server) {
        return parseClientMaxBodySize(tokens, pos, &server->client_max_body_size);
    } else if (location) {
        return parseClientMaxBodySize(tokens, pos, &location->client_max_body_size);
    }
    return false;
}

bool DirectiveParser::handleListenDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        toolbox::logger::StepMark::error("\"" + std::string(config::directive::LISTEN) + "\" directive is not allowed here");
    } else if (server) {
        return parseListenDirective(tokens, pos, &server->listens);
    } else if (location) {
        toolbox::logger::StepMark::error("\"" + std::string(config::directive::LISTEN) + "\" directive is not allowed here");
    }
    return false;
}

bool DirectiveParser::handleServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* http, config::ServerConfig* server, config::LocationConfig* location) {
    if (http) {
        toolbox::logger::StepMark::error("\"" + std::string(config::directive::SERVER_NAME) + "\" directive is not allowed here");
    } else if (server) {
        return parseServerNameDirective(tokens, pos, &server->server_names);
    } else if (location) {
        toolbox::logger::StepMark::error("\"" + std::string(config::directive::SERVER_NAME) + "\" directive is not allowed here");
    }
    return false;
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
        toolbox::logger::StepMark::error("\"" + directive_name + "\" directive is duplicate");
        return false;
    }
}

bool DirectiveParser::isAllowedDuplicate(const std::string& directive_name) {
    static const std::string allowed_duplicates[] = {
        config::directive::ALLOWED_METHODS,
        config::directive::CGI_EXTENSION,
        config::directive::ERROR_PAGE,
        config::directive::INDEX,
        config::directive::SERVER_NAME,
    };
    static const size_t allowed_count = sizeof(allowed_duplicates) / sizeof(allowed_duplicates[0]);

    for (size_t i = 0; i < allowed_count; ++i) {
        if (directive_name == allowed_duplicates[i]) {
            return true;
        }
    }
    return false;
}

bool DirectiveParser::isIgnoredDuplicate(const std::string& directive_name) {
    static const std::string ignored_duplicates[] = {
        config::directive::RETURN
    };
    static const size_t ignored_count = sizeof(ignored_duplicates) / sizeof(ignored_duplicates[0]);

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
