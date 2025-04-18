// Copyright 2025 Ideal Broccoli

#pragma once

#include <ctime>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cstdlib>

#include "../../../toolbox/string.hpp"
#include "http_request.hpp"
#include "../http_namespace.hpp"

namespace http {
class RequestParser {
 public:
    RequestParser() : _validatePos(REQUEST_LINE), _time(0) {}
    ~RequestParser() {}
    class ParseException : public std::exception {
     public:
        explicit ParseException(const char* message);
        const char* what() const throw();

     private:
        const char* _message;
    };
    enum ParseState {
        REQUEST_LINE  = 0,
        HEADERS     = 1,
        BODY        = 2,
        COMPLETED   = 3,
        ERROR       = 4
    };
    void run(const std::string& buf);
    HTTPRequest& get() { return _request; }

 private:
    RequestParser(const RequestParser& other);
    RequestParser& operator=(const RequestParser& other);
    void parseRequestLine();
    void parseURI();
    void parseFields();
    void parseBody();
    void parseChunkedEncoding();
    void validateFieldLine(std::string& line);
    void validateMethod();
    void validateURI();
    void validateVersion();
    void urlDecode();

    std::string _buf;
    HTTPRequest _request;
    ParseState _validatePos;
    std::time_t _time;
};

bool isDigitStr(const std::string& str);
bool hasWhiteSpace(const std::string& str);
}  // namespace http
