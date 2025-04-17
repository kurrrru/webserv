// Copyright 2025 Ideal Broccoli

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../toolbox/stepmark.hpp"

namespace config {

DirectiveParser::DirectiveParser() {
    initDirectiveInfo();
}

// ディレクティブの初期化
void DirectiveParser::initDirectiveInfo() {
    DirectiveInfo info;

    info.directive = config::directive::DIRECTIVE_ROOT;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_ROOT] = info;

    info.directive = config::directive::DIRECTIVE_INDEX;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_INDEX] = info;

    info.directive = config::directive::DIRECTIVE_AUTOINDEX;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_AUTOINDEX] = info;

    info.directive = config::directive::DIRECTIVE_ALLOWED_METHODS;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_ALLOWED_METHODS] = info;

    info.directive = config::directive::DIRECTIVE_ERROR_PAGE;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_ERROR_PAGE] = info;

    info.directive = config::directive::DIRECTIVE_UPLOAD_STORE;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_UPLOAD_STORE] = info;

    info.directive = config::directive::DIRECTIVE_CGI_PASS;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_CGI_PASS] = info;

    info.directive = config::directive::DIRECTIVE_CGI_EXTENSION;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_CGI_EXTENSION] = info;

    info.directive = config::directive::DIRECTIVE_CLIENT_MAX_BODY_SIZE;
    info.context = CONTEXT_ALL;
    _directive_info[config::directive::DIRECTIVE_CLIENT_MAX_BODY_SIZE] = info;

    info.directive = config::directive::DIRECTIVE_LISTEN;
    info.context = CONTEXT_SERVER;
    _directive_info[config::directive::DIRECTIVE_LISTEN] = info;

    info.directive = config::directive::DIRECTIVE_SERVER_NAME;
    info.context = CONTEXT_SERVER;
    _directive_info[config::directive::DIRECTIVE_SERVER_NAME] = info;

    info.directive = config::directive::DIRECTIVE_RETURN;
    info.context = CONTEXT_SERVER_LOCATION;
    _directive_info[config::directive::DIRECTIVE_RETURN] = info;
}

bool DirectiveParser::parseDirective(const std::vector<std::string>& tokens, size_t* pos, const std::string& directive, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    // TODO(yootsubo) :チェックが必要かもかも知らないところで使われるとアウト
    // 各ディレクティブに対応するハンドラー関数を呼び出す
    if (directive == config::directive::DIRECTIVE_ROOT) {
        return handleRootDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_INDEX) {
        return handleIndexDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_AUTOINDEX) {
        return handleAutoindexDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_ALLOWED_METHODS) {
        return handleAllowedMethodsDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_ERROR_PAGE) {
        return handleErrorPageDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_UPLOAD_STORE) {
        return handleUploadStoreDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_CGI_PASS) {
        return handleCgiPassDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_CGI_EXTENSION) {
        return handleCgiExtensionDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_RETURN) {
        return handleReturnDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_CLIENT_MAX_BODY_SIZE) {
        return handleClientMaxBodySizeDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_LISTEN) {
        return handleListenDirective(tokens, pos, server, location, http);
    } else if (directive == config::directive::DIRECTIVE_SERVER_NAME) {
        return handleServerNameDirective(tokens, pos, server, location, http);
    // 未知のディレクティブ
    } else {
        toolbox::logger::StepMark::error("Unknown directive '" + directive + "'");
        return false;
    }
    // ここに来るのは内部エラー
    toolbox::logger::StepMark::error("Internal Error: Directive '" + directive + "' was allowed but no handler was executed.");
    return false;
}

// ディレクティブのコンテキストを取得
DirectiveContext DirectiveParser::getCurrentContext(config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* config) const {
    if (location) {
        return CONTEXT_LOCATION;
    }
    if (server) {
        return CONTEXT_SERVER;
    }
    if (config) {
        return CONTEXT_HTTP;
    }
    return CONTEXT_NONE;
}

// ディレクティブが現在のコンテキストで許可されているかチェック
bool DirectiveParser::isDirectiveAllowedInContext(const std::string& directive, DirectiveContext context) const {
    // ディレクティブ情報を取得
    std::map<std::string, DirectiveInfo>::const_iterator it = _directive_info.find(directive);
    if (it == _directive_info.end()) {
        return false;
    }
    // DirectiveInfo構造体のコンテキストと引数のコンテキストを2進数のビット演算比較
    return (it->second.context & context) != 0;
}

bool DirectiveParser::handleAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseAllowedMethodsDirective(tokens, pos, &location->allowed_methods); 
    } else if (server) {
        return parseAllowedMethodsDirective(tokens, pos, &server->allowed_methods); 
    } else if (http) {
        return parseAllowedMethodsDirective(tokens, pos, &http->allowed_methods); 
    }
    return false;
}

bool DirectiveParser::handleAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseAutoindexDirective(tokens, pos, &location->autoindex); 
    } else if (server) {
        return parseAutoindexDirective(tokens, pos, &server->autoindex); 
    } else if (http) {
        return parseAutoindexDirective(tokens, pos, &http->autoindex); 
    }
    return false;
}

bool DirectiveParser::handleRootDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseRootDirective(tokens, pos, &location->root); 
    } else if (server) {
        return parseRootDirective(tokens, pos, &server->root); 
    } else if (http) {
        return parseRootDirective(tokens, pos, &http->root); 
    }
    return false;
}

bool DirectiveParser::handleIndexDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseIndexDirective(tokens, pos, &location->index); 
    } else if (server) {
        return parseIndexDirective(tokens, pos, &server->index); 
    } else if (http) {
        return parseIndexDirective(tokens, pos, &http->index); 
    }
    return false;
}

bool DirectiveParser::handleErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseErrorPageDirective(tokens, pos, &location->error_pages); 
    } else if (server) {
        return parseErrorPageDirective(tokens, pos, &server->error_pages); 
    } else if (http) {
        return parseErrorPageDirective(tokens, pos, &http->error_pages); 
    }
    return false;
}

bool DirectiveParser::handleUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseUploadStoreDirective(tokens, pos, &location->upload_store); 
    } else if (server) {
        return parseUploadStoreDirective(tokens, pos, &server->upload_store); 
    } else if (http) {
        return parseUploadStoreDirective(tokens, pos, &http->upload_store); 
    }
    return false;
}

bool DirectiveParser::handleCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseCgiPassDirective(tokens, pos, &location->cgi_pass); 
    } else if (server) {
        return parseCgiPassDirective(tokens, pos, &server->cgi_pass); 
    } else if (http) {
        return parseCgiPassDirective(tokens, pos, &http->cgi_pass); 
    }
    return false;
}

bool DirectiveParser::handleCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseCgiExtensionDirective(tokens, pos, &location->cgi_extension); 
    } else if (server) {
        return parseCgiExtensionDirective(tokens, pos, &server->cgi_extension); 
    } else if (http) {
        return parseCgiExtensionDirective(tokens, pos, &http->cgi_extension); 
    }
    return false;
}

bool DirectiveParser::handleReturnDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (location) {
        return parseReturnDirective(tokens, pos, &location->return_code, &location->return_url); 
    } else if (server) {
        return parseReturnDirective(tokens, pos, &server->return_code, &server->return_url); 
    } else if (http) {
        return parseReturnDirective(tokens, pos, &http->return_code, &http->return_url); 
    }
    return false;
}

// 専用ディレクティブのハンドラー関数
bool DirectiveParser::handleClientMaxBodySizeDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (http) {
        return parseClientMaxBodySize(tokens, pos, &http->client_max_body_size);
    } else if (server) {
        return parseClientMaxBodySize(tokens, pos, &server->client_max_body_size);
    } else if (location) {
        return parseClientMaxBodySize(tokens, pos, &location->client_max_body_size);
    }
    return false;
}

bool DirectiveParser::handleListenDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (http) {
        toolbox::logger::StepMark::error("listen directive is not allowed in http context");
    } else if (server) {
        return parseListenDirective(tokens, pos, &server->port);
    } else if (location) {
        toolbox::logger::StepMark::error("listen directive is not allowed in location context");
    }
    return false;
}

bool DirectiveParser::handleServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server, config::LocationConfig* location, config::HttpConfig* http) {
    if (http) {
        toolbox::logger::StepMark::error("server_name directive is not allowed in http context");
    } else if (server) {
        return parseServerNameDirective(tokens, pos, &server->server_names);
    } else if (location) {
        toolbox::logger::StepMark::error("server_name directive is not allowed in location context");
    }
    return false;
}
}  // namespace config
