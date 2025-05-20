#include <string>
#include <vector>

#include "config_namespace.hpp"

namespace config {

namespace token {
// Token types
const char* OPEN_BRACE = "{";
const char* CLOSE_BRACE = "}";
const char* SEMICOLON = ";";
const char* COLON = ":";
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
const char* ALLOWED_METHODS = "allowed_methods";
const char* AUTOINDEX = "autoindex";
const char* CGI_EXTENSION = "cgi_extension";
const char* CGI_PATH = "cgi_path";
const char* CLIENT_MAX_BODY_SIZE = "client_max_body_size";
const char* ERROR_PAGE = "error_page";
const char* INDEX = "index";
const char* LISTEN = "listen";
const char* RETURN = "return";
const char* ROOT = "root";
const char* SERVER_NAME = "server_name";
const char* UPLOAD_STORE = "upload_store";
const char* SEMICOLON = ";";
const char* ON = "on";
const char* OFF = "off";
const size_t MIN_ERROR_PAGE_CODE = 300;
const size_t MAX_ERROR_PAGE_CODE = 599;
const char* ASTERISK = "*";
const size_t MIN_PORT = 1;
const size_t MAX_PORT = 65535;
const char* LISTEN_DEFAULT_SERVER = "default_server";
const size_t MAX_RETURN_CODE = 999;
}  // namespace directive

namespace method {
const char* GET = "GET";
const char* HEAD = "HEAD";
const char* POST = "POST";
const char* DELETE = "DELETE";
const size_t ALLOWED_METHODS_COUNT = 4;
const char* ALLOWED_METHODS[ALLOWED_METHODS_COUNT] = {GET, HEAD, POST, DELETE};
}  // namespace method

const bool DEFAULT_AUTOINDEX = false;
const size_t DEFAULT_CLIENT_MAX_BODY_SIZE = 1024 * 1024;
const char* DEFAULT_CGI_PATH = "cgi";
const char* DEFAULT_FILE = "conf/default.conf";
const int DEFAULT_PORT = 80;
const std::vector<std::string> DEFAULT_INDICES(1, "index.html");
const char* DEFAULT_INDEX = "index.html";
const char* DEFAULT_IP = "0.0.0.0";
const char* DEFAULT_ROOT = "html";
const char* DEFAULT_SERVER_NAME = "";
const char* DEFAULT_UPLOAD_STORE = "upload";
const char* DEFAULT_LOCATION_PATH = "/";
const size_t CONF_BUFFER = 4096;

}  // namespace config
