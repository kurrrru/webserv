// Copyright 2025 Ideal Broccoli

#include <string>
#include <vector>

#include "config_namespace.hpp"

namespace config {

namespace token {
// Token types
const char* OPEN_BRACE = "{";
const char* CLOSE_BRACE = "}";
const char* SEMICOLON = ";";
const char* LF = "\n";
const char* CR = "\r";
const char* TAB = "\t";
const char* BACKSLASH = "\\";
const char* COMMENT_CHAR = "#";
const char* DOUBLE_QUOTE = "\"";
const char* SINGLE_QUOTE = "'";
const char* PERIOD = ".";
const char* SLASH = "/";
const char* WHITESPACE_CHARS = " \t\n\r";
}  // namespace token

namespace context {
const char* HTTP = "http";
const char* SERVER = "server";
const char* LOCATION = "location";
}  // namespace context

namespace directive {
const char* CLIENT_MAX_BODY_SIZE = "client_max_body_size";
const char* LISTEN = "listen";
const char* SERVER_NAME = "server_name";
const char* ROOT = "root";
const char* INDEX = "index";
const char* ALLOWED_METHODS = "allowed_methods";
const char* UPLOAD_STORE = "upload_store";
const char* CGI_PASS = "cgi_pass";
const char* ERROR_PAGE = "error_page";
const char* AUTOINDEX = "autoindex";
const char* CGI_EXTENSION = "cgi_extension";
const char* RETURN = "return";
const char* SEMICOLON = ";";
const char* ON = "on";
const char* OFF = "off";
const size_t MIN_ERROR_PAGE_CODE = 300;
const size_t MAX_ERROR_PAGE_CODE = 599;
}  // namespace directive

namespace method {
const char* GET = "GET";
const char* POST = "POST";
const char* DELETE = "DELETE";
const size_t ALLOWED_METHODS_COUNT = 3;
const char* ALLOWED_METHODS[ALLOWED_METHODS_COUNT] = {GET, POST, DELETE};
}  // namespace method

const bool DEFAULT_AUTOINDEX = false;
const size_t DEFAULT_CLIENT_MAX_BODY_SIZE = 1024 * 1024;
const char* DEFAULT_CGI_PATH = "cgi";
const char* DEFAULT_FILE = "conf/default.conf";
const int DEFAULT_PORT = 80;
const std::vector<std::string> DEFAULT_INDICES(1, "index.html");
const char* DEFAULT_IP = "0.0.0.0";
const char* DEFAULT_ROOT = "html";
const std::vector<std::string> DEFAULT_SERVER_NAME(1, "");
const char* DEFAULT_UPLOAD_STORE = "upload";
const char* DEFAULT_LOCATION_PATH = "/";
const size_t CONF_BUFFER = 4096;

}  // namespace config
