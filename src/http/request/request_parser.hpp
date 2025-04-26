// Copyright 2025 Ideal Broccoli

#pragma once

#include <ctime>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cstdlib>

#include "../../../toolbox/string.hpp"
#include "../http_namespace.hpp"
#include "../http_status.hpp"
#include "http_request.hpp"

namespace http {
class RequestParser {
 public:
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

    RequestParser() : _validatePos(REQUEST_LINE), _time(0) {}
    ~RequestParser() {}
    void run(const std::string& buf);
    HTTPRequest& get() { return _request; }

 private:
    RequestParser(const RequestParser& other);
    RequestParser& operator=(const RequestParser& other);

    void processRequestLine();
        void parseRequestLine();
        void validateMethod();
        void processURI();
            void parseURI();
            void urlDecode();
            void validatePath();
            void parseQuery();
        void validateVersion();

    void processFields();
        void validateFieldLine(std::string& line);
        void parseChunkedEncoding();

    void processBody();

    std::string _buf;
    HTTPRequest _request;
    ParseState _validatePos;
    std::time_t _time;
};

bool hasCtlChar(const std::string& str);
void logInfo(HttpStatus status, const std::string& message);

}  // namespace http
