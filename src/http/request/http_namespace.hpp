#pragma once

#include <string>

namespace http {
    //method
    const std::string GET = "GET";
    const std::string POST = "POST";
    const std::string DELETE = "DELETE";
    //version
    const std::string HTTP_VERSION = "HTTP/1.1";
    //fields
    const std::string Host = "Host";
    const std::string ContentType = "Content-Type";
    const std::string ContentLength = "Content-Length";
    const std::string UserAgent = "User-Agent";
    const std::string Connection = "Connection";
    const std::string Accept = "Accept";
    //other
    const std::string CRLF = "\r\n";
    const std::string SP = " ";
}