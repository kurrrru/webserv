#include "http_namespace.hpp"

namespace http {
// method
namespace method {
const char* GET = "GET";
const char* HEAD = "HEAD";
const char* POST = "POST";
const char* DELETE = "DELETE";
}  // namespace method

namespace uri {
const char* HTTP_VERSION_1_1 = "HTTP/1.1";
const std::size_t HTTP_MAJOR_VERSION = 1;
const std::size_t HTTP_MINOR_VERSION_MAX = 999;
const std::size_t MAX_URI_SIZE = 8192;
}

namespace fields {
// common
const char* DATE = "Date";
const char* CACHE_CONTROL = "Cache-Control";
const char* CONNECTION = "Connection";
const char* CONTENT_LENGTH = "Content-Length";
const char* CONTENT_TYPE = "Content-Type";
const char* CONTENT_ENCODING = "Content-Encoding";
const char* CONTENT_LANGUAGE = "Content-Language";
const char* TRANSFER_ENCODING = "Transfer-Encoding";
// request fields
const char* HOST = "Host";
const char* ACCEPT = "Accept";
const char* ACCEPT_LANGUAGE = "Accept-Language";
const char* ACCEPT_ENCODING = "Accept-Encoding";
const char* AUTHORIZATION = "Authorization";
const char* USER_AGENT = "User-Agent";
const char* COOKIE = "Cookie";
const char* REFERER = "Referer";
// response fields
const char* SERVER = "Server";
const char* SET_COOKIE = "Set-Cookie";
const char* LOCATION = "Location";
const char* WWW_AUTHENTICATE = "WWW-Authenticate";
const char* LAST_MODIFIED = "Last-Modified";
const char* FIELDS[] = {
DATE,          CACHE_CONTROL,    CONNECTION,       CONTENT_LENGTH,
CONTENT_TYPE,  CONTENT_ENCODING, CONTENT_LANGUAGE, TRANSFER_ENCODING,
HOST,          ACCEPT,           ACCEPT_ENCODING,  ACCEPT_LANGUAGE,
AUTHORIZATION, USER_AGENT,       COOKIE,           REFERER,
SERVER,        SET_COOKIE,       LOCATION,         WWW_AUTHENTICATE,
LAST_MODIFIED
};
const std::size_t FIELD_SIZE = sizeof(FIELDS) / sizeof(FIELDS[0]);
const std::size_t MAX_FIELDLINE_SIZE = 8192;
namespace cgi {
const char* STATUS = "Status";
}
}  // namespace fields

namespace symbols {
const char* CR = "\r";
const char* LF = "\n";
const char* CRLF = "\r\n";
const char* SP = " ";
const char* EQUAL = "=";
const char* AMPERSAND = "&";
const char* HASH = "#";
const char* SLASH = "/";
const char* QUESTION = "?";
const char* PERCENT = "%";
const char* COLON = ":";
const char* SEMICOLON = ";";
const char* COMMA = ",";
const char* COMMASP = ", ";
const std::size_t LF_SIZE = 1;
const std::size_t CRLF_SIZE = 2;
}  // namespace symbols

namespace cgi {
const size_t MAX_REDIRECTS = 10;
const size_t TIMEOUT = 30;
const size_t READ_BUFFER_SIZE = 4096;
const size_t READ_TIMEOUT_SEC = 1;
const char* GATEWAY_INTERFACE = "CGI/1.1";
const char* SERVER_SOFTWARE = "WebServ/1.0";
const char* ENV_PREFIX = "HTTP_";
namespace meta {
const char* AUTH_TYPE = "AUTH_TYPE";
const char* CONTENT_LENGTH = "CONTENT_LENGTH";
const char* CONTENT_TYPE = "CONTENT_TYPE";
const char* GATEWAY_INTERFACE = "GATEWAY_INTERFACE";
const char* PATH_INFO = "PATH_INFO";
const char* PATH_TRANSLATED = "PATH_TRANSLATED";
const char* QUERY_STRING = "QUERY_STRING";
const char* REMOTE_ADDR = "REMOTE_ADDR";
const char* REMOTE_HOST = "REMOTE_HOST";
const char* REMOTE_IDENT = "REMOTE_IDENT";
const char* REMOTE_USER = "REMOTE_USER";
const char* REQUEST_METHOD = "REQUEST_METHOD";
const char* SCRIPT_NAME = "SCRIPT_NAME";
const char* SERVER_NAME = "SERVER_NAME";
const char* SERVER_PORT = "SERVER_PORT";
const char* SERVER_PROTOCOL = "SERVER_PROTOCOL";
const char* SERVER_SOFTWARE = "SERVER_SOFTWARE";
const char* UPLOAD_DIR = "UPLOAD_DIR";
}  // namespace meta
}  // namespace cgi
}  // namespace http
