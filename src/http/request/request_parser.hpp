#pragma once

#include "http_request.hpp"

class RequestParser {
   public:
    enum ParseState { REQUEST_LINE, HEADERS, BODY, COMPLETED, ERROR };
    void run(const std::string& buf);
    HTTPRequest get();

   private:
    HTTPRequest _request;
    std::string _buf;
    ParseState _state;

    void parseRequestLine();
    void parseFields();
    void parseBody();

    void validateMethod();
    void validateURI();
    void validateVersion();
};