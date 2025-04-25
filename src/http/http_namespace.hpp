// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <utility>

namespace http {
// method
namespace method {
extern const char* GET;
extern const char* HEAD;
extern const char* POST;
extern const char* DELETE;
}  // namespace method

namespace version {
extern const char* HTTP_VERSION;
}

namespace fields {
// common
extern const char* DATE;
extern const char* CACHE_CONTROL;
extern const char* CONNECTION;
extern const char* CONTENT_LENGTH;
extern const char* CONTENT_TYPE;
extern const char* CONTENT_ENCODING;
extern const char* CONTENT_LANGUAGE;
extern const char* TRANSFER_ENCODING;
// request fields
extern const char* HOST;
extern const char* ACCEPT;
extern const char* ACCEPT_LANGUAGE;
extern const char* ACCEPT_ENCODING;
extern const char* AUTHORIZATION;
extern const char* USER_AGENT;
extern const char* COOKIE;
// response fields
extern const char* SERVER;
extern const char* SET_COOKIE;
extern const char* LOCATION;
extern const char* WWW_AUTHENTICATE;
extern const char* FIELDS[];
extern const std::size_t FIELD_SIZE;
extern const std::size_t MAX_FIELDLINE_SIZE;
extern const std::size_t MAX_BODY_SIZE;
}  // namespace fields

namespace symbols {
extern const char* CR;
extern const char* CRLF;
extern const char* SP;
extern const char* EQUAL;
extern const char* AMPERSAND;
extern const char* HASH;
extern const char* SLASH;
extern const char* QUESTION;
extern const char* PERCENT;
extern const char* COLON;
extern const char* COMMA;
extern const char* COMMASP;
}  // namespace symbols

}  // namespace http
