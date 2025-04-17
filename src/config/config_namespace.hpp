// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>
#include <set>

namespace config {

namespace token {
// Token types
const char OPEN_BRACE[] = "{";
const char CLOSE_BRACE[] = "}";
const char SEMICOLON[] = ";";

// Token delimiters
const char WHITESPACE_CHARS[] = " \t\n\r";
const char COMMENT_CHAR = '#';
const char DOUBLE_QUOTE = '"';
const char SINGLE_QUOTE = '\'';
}  // namespace token

namespace context {
const char CONTEXT_HTTP[] = "http";
const char CONTEXT_SERVER[] = "server";
const char CONTEXT_LOCATION[] = "location";
}  // namespace context

namespace directive {
const char DIRECTIVE_CLIENT_MAX_BODY_SIZE[] = "client_max_body_size";
const char DIRECTIVE_LISTEN[] = "listen";
const char DIRECTIVE_SERVER_NAME[] = "server_name";
const char DIRECTIVE_ROOT[] = "root";
const char DIRECTIVE_INDEX[] = "index";
const char DIRECTIVE_ALLOWED_METHODS[] = "allowed_methods";
const char DIRECTIVE_UPLOAD_STORE[] = "upload_store";
const char DIRECTIVE_CGI_PASS[] = "cgi_pass";
const char DIRECTIVE_ERROR_PAGE[] = "error_page";
const char DIRECTIVE_AUTOINDEX[] = "autoindex";
const char DIRECTIVE_CGI_EXTENSION[] = "cgi_extension";
const char DIRECTIVE_RETURN[] = "return";
}  // namespace directive

namespace method {
const char METHOD_GET[] = "GET";
const char METHOD_POST[] = "POST";
const char METHOD_DELETE[] = "DELETE";
const char METHOD_HEAD[] = "HEAD";
}  // namespace method

const int DEFAULT_PORT = 80;
const size_t DEFAULT_CLIENT_MAX_BODY_SIZE = 1024 * 1024;
const char DEFAULT_FILE[] = "/conf/default.conf";

struct ErrorPage {
    std::vector<int> codes;
    std::string path;

    ErrorPage()
        : codes(),
        path() {}
};

struct LocationConfig {
    std::vector<std::string> allowed_methods;
    bool autoindex;
    std::string cgi_extension;
    std::string cgi_pass;
    size_t client_max_body_size;
    std::vector<ErrorPage> error_pages;
    std::vector<std::string> index;
    std::string path;
    int return_code;
    std::string return_url;
    std::string root;
    std::string upload_store;
    std::set<std::string> parsedDirectives;

    LocationConfig()
        : allowed_methods(),
        autoindex(false),
        cgi_extension(""),
        cgi_pass(""),
        client_max_body_size(DEFAULT_CLIENT_MAX_BODY_SIZE),
        error_pages(),
        index(),
        path(""),
        return_code(-1),
        return_url(""),
        root(""),
        upload_store(""),
        parsedDirectives()
        {}
};

struct ServerConfig {
    std::vector<std::string> allowed_methods;
    bool autoindex;
    std::string cgi_extension;
    std::string cgi_pass;
    size_t client_max_body_size;
    std::vector<ErrorPage> error_pages;
    std::vector<std::string> index;
    int port;
    int return_code;
    std::string return_url;
    std::string root;
    std::vector<std::string> server_names;
    std::string upload_store;
    std::vector<LocationConfig> locations;
    std::set<std::string> parsedDirectives;

    ServerConfig()
        : allowed_methods(),
        autoindex(false),
        cgi_extension(""),
        cgi_pass(""),
        client_max_body_size(DEFAULT_CLIENT_MAX_BODY_SIZE),
        error_pages(),
        index(),
        port(DEFAULT_PORT),
        return_code(-1),
        return_url(""),
        root(""),
        server_names(),
        upload_store(""),
        locations(),
        parsedDirectives()
        {}
};

struct HttpConfig {
    std::vector<std::string> allowed_methods;
    bool autoindex;
    std::string cgi_extension;
    std::string cgi_pass;
    size_t client_max_body_size;
    std::vector<ErrorPage> error_pages;
    std::vector<std::string> index;
    int return_code;
    std::string return_url;
    std::string root;
    std::string upload_store;
    std::vector<ServerConfig> servers;
    std::set<std::string> parsedDirectives;

    HttpConfig()
        : allowed_methods(),
        autoindex(false),
        cgi_extension(""),
        cgi_pass(""),
        client_max_body_size(DEFAULT_CLIENT_MAX_BODY_SIZE),
        error_pages(),
        index(),
        return_code(-1),
        return_url(""),
        root(""),
        upload_store(""),
        servers(),
        parsedDirectives()
        {}
};

}  // namespace config
