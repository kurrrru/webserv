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
extern const char* COLON;
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
extern const char* ALLOWED_METHODS;
extern const char* AUTOINDEX;
extern const char* CGI_EXTENSION;
extern const char* CGI_PATH;
extern const char* CLIENT_MAX_BODY_SIZE;
extern const char* ERROR_PAGE;
extern const char* INDEX;
extern const char* LISTEN;
extern const char* RETURN;
extern const char* ROOT;
extern const char* SERVER_NAME;
extern const char* UPLOAD_STORE;
extern const char* SEMICOLON;
extern const char EQUAL;
extern const char* ON;
extern const char* OFF;
extern const std::size_t MIN_ERROR_PAGE_CODE;
extern const std::size_t MAX_ERROR_PAGE_CODE;
extern const std::size_t MIN_NEW_STATUS_CODE;
extern const std::size_t MAX_NEW_STATUS_CODE;
extern const char* ASTERISK;
extern const std::size_t MIN_PORT;
extern const std::size_t MAX_PORT;
extern const char* LISTEN_DEFAULT_SERVER;
extern const std::size_t MAX_RETURN_CODE;
}  // namespace directive

namespace method {
extern const char* GET;
extern const char* HEAD;
extern const char* POST;
extern const char* DELETE;
extern const std::size_t ALLOWED_METHODS_COUNT;
extern const char* ALLOWED_METHODS[];
}  // namespace method

extern const bool DEFAULT_AUTOINDEX;
extern const std::size_t DEFAULT_CLIENT_MAX_BODY_SIZE;
extern const char* DEFAULT_CGI_PATH;
extern const char* DEFAULT_FILE;
extern const int DEFAULT_PORT;
extern const std::vector<std::string> DEFAULT_INDICES;
extern const char* DEFAULT_INDEX;
extern const char* DEFAULT_IP;
extern const char* DEFAULT_ROOT;
extern const char* DEFAULT_SERVER_NAME;
extern const char* DEFAULT_UPLOAD_STORE;
extern const char* DEFAULT_LOCATION_PATH;
extern const std::size_t CONF_BUFFER;
}  // namespace config
