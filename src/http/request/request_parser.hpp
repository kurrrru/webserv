#pragma once

#include "http_request.hpp"

class RequestParser {
   public:
    RequestParser() : _state(REQUEST_LINE) {};
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
    HTTPRequest& get() { return _request; }; //debug

   private:
    RequestParser(RequestParser& other) {};
    RequestParser& operator=(RequestParser& other) { return *this; };
    void parseRequestLine();
    void parseURI();
    void parseFields();
    void parseBody();
    void validateMethod();
    void validateURI();
    void validateVersion();


    std::string _buf;
    HTTPRequest _request;
    ParseState _state;
    std::pair<std::string, std::string> _requestState;
};
