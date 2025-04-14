#pragma once

#include <ctime>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cstdlib>

#include "../../../toolbox/string.hpp"
#include "http_request.hpp"

namespace http {
class RequestParser {
   public:
    RequestParser() : _state(REQUEST_LINE), _time(0) {};
    ~RequestParser() {};
    class ParseException : public std::exception {
       public:
        ParseException(const char* message);
        const char* what() const throw();

       private:
        const char* _message;
    };
    enum ParseState { REQUEST_LINE, HEADERS, BODY, COMPLETED, ERROR };
    void run(const std::string& buf);
    HTTPRequest& get() { return _request; };

   private:
    RequestParser(RequestParser& other);
    RequestParser& operator=(RequestParser& other);
    void parseRequestLine();
    void parseURI();
    void parseFields();
    void parseBody();
    void parseChunkedEncoding();
    void validateMethod();
    void validateURI();
    void validateVersion();
    void urlDecode();

    std::string _buf;
    HTTPRequest _request;
    ParseState _state;
    std::pair<std::string, std::string> _requestState;
    std::time_t _time;
};
}  // namespace http
