// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>

namespace config {

namespace token {
// Token types
extern const char* OPEN_BRACE;
extern const char* CLOSE_BRACE;
extern const char* SEMICOLON;
extern const char* LF;
extern const char* CR;
extern const char* TAB;
extern const char* BACKSLASH;
extern const char* COMMENT_CHAR;
extern const char* DOUBLE_QUOTE;
extern const char* SINGLE_QUOTE;
extern const char* PERIOD;
extern const char* SLASH;
extern const char* WHITESPACE_CHARS;
}  // namespace token

namespace context {
extern const char* HTTP;
extern const char* SERVER;
extern const char* LOCATION;
}  // namespace context

namespace directive {
extern const char* CLIENT_MAX_BODY_SIZE;
extern const char* LISTEN;
extern const char* SERVER_NAME;
extern const char* ROOT;
extern const char* INDEX;
extern const char* ALLOWED_METHODS;
extern const char* UPLOAD_STORE;
extern const char* CGI_PASS;
extern const char* ERROR_PAGE;
extern const char* AUTOINDEX;
extern const char* CGI_EXTENSION;
extern const char* RETURN;
extern const char* SEMICOLON;
extern const char* ON;
extern const char* OFF;
extern const size_t MIN_ERROR_PAGE_CODE;
extern const size_t MAX_ERROR_PAGE_CODE;
}  // namespace directive

namespace method {
extern const char* GET;
extern const char* POST;
extern const char* DELETE;
extern const size_t ALLOWED_METHODS_COUNT;
extern const char* ALLOWED_METHODS[];
}  // namespace method

extern const bool DEFAULT_AUTOINDEX;
extern const size_t DEFAULT_CLIENT_MAX_BODY_SIZE;
extern const char* DEFAULT_CGI_PATH;
extern const char* DEFAULT_FILE;
extern const int DEFAULT_PORT;
extern const std::vector<std::string> DEFAULT_INDICES;
extern const char* DEFAULT_IP;
extern const char* DEFAULT_ROOT;
extern const std::vector<std::string> DEFAULT_SERVER_NAME;
extern const char* DEFAULT_UPLOAD_STORE;
extern const char* DEFAULT_LOCATION_PATH;
extern const size_t CONF_BUFFER;

struct ErrorPage {
    std::vector<size_t> codes;
    std::string path;
    ErrorPage()
        : codes(),
        path() {}
};

struct Listen {
    size_t port;
    std::string ip;
    bool default_server;
    Listen()
        : port(DEFAULT_PORT),
        ip(DEFAULT_IP),
        default_server(false) {}
};

struct ServerName {
    enum ServerNameType {
        EXACT,
        WILDCARD_START,
        WILDCARD_END
    };
    std::vector<std::string> names;
    ServerNameType type;
    ServerName()
        : names(DEFAULT_SERVER_NAME),
        type(EXACT) {}
};

struct Return {
    size_t status_code;
    std::string text_or_url;
    bool has_return_setting;
    Return()
        : status_code(),
        text_or_url(),
        has_return_setting(false) {}
};


}  // namespace config
