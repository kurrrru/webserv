#pragma once

#include <string>

namespace http {
// method
const std::string GET = "GET";
const std::string POST = "POST";
const std::string DELETE = "DELETE";
// http version
const std::string HTTP_VERSION = "HTTP/1.1";
// common fields
const std::string DATE = "Date";
const std::string CACHE_CONTROL = "Cache-Control";
const std::string CONNECTION = "Connection";
const std::string CONTENT_LENGTH = "Content-Length";
const std::string CONTENT_TYPE = "Content-Type";
const std::string CONTENT_ENCODING = "Content-Encoding";
const std::string CONTENT_LANGUAGE = "Content-Language";
const std::string TRANSFER_ENCODING = "Transfer-Encoding";
// request fields
const std::string HOST = "Host";
const std::string ACCEPT = "Accept";
const std::string ACCEPT_LANGUAGE = "Accept-Language";
const std::string ACCEPT_ENCODING = "Accept-Encoding";
const std::string AUTHORIZATION = "Authorization";
const std::string USER_AGENT = "User-Agent";
const std::string COOKIE = "Cookie";
// response fields
const std::string SERVER = "Server";
const std::string SET_COOKIE = "Set-Cookie";
const std::string LOCATION = "Location";
const std::string WWW_AUTHENTICATE = "WWW-Authenticate";
const std::string FIELDS[] = {
    DATE,          CACHE_CONTROL,    CONNECTION,       CONTENT_LENGTH,
    CONTENT_TYPE,  CONTENT_ENCODING, CONTENT_LANGUAGE, TRANSFER_ENCODING,
    HOST,          ACCEPT,           ACCEPT_ENCODING,  ACCEPT_LANGUAGE,
    AUTHORIZATION, USER_AGENT,       COOKIE,           SERVER,
    SET_COOKIE,    LOCATION,         WWW_AUTHENTICATE};
const std::size_t FIELD_SIZE = sizeof(FIELDS) / sizeof(FIELDS[0]);
// other
const std::string CRLF = "\r\n";
const std::string SP = " ";
const std::string EQUAL = "=";
const std::string AMPERSAND = "&";
const std::string HASH = "#";
const std::string QUESTION = "?";
}  // namespace http
