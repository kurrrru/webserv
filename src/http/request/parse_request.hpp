#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "../http_namespace.hpp"
#include "../status_code.hpp"
#include "parse_request.hpp"

enum RequestState { REQUEST_LINE, HEADERS, BODY, COMPLETED };

enum ParseState {
    PARSE_INVALID_METHOD,
    PARSE_INVALID_URI,
    PARSE_INVALID_VERSION
};

struct RequestLine {
    std::string method;
    std::string uri;
    std::string path;
    std::string query;
    std::map<std::string, std::string> queryMap;
    std::string fragment;  // no processing required

    std::string version;
};

struct Fields {
    void initField();
    const std::string& get(const std::string& key) const;
    bool set(std::pair<std::string, std::vector<std::string>>& pair);

    std::map<std::string, std::vector<std::string>> fields;
};

struct Body {
    Body() : contentLength(0), recvedLength(0) {}
    std::size_t contentLength;
    std::size_t recvedLength;
    std::string body;
};

struct RequestData {
    RequestData() : requestState(0), hasError(0), errorCode(0) {}
    int requestState;
    bool hasError;
    int errorCode;
    std::pair<std::string, std::string> errorStatus;
    RequestLine requestLine;
    Fields field;
    Body body;
    std::string inputBuffer;
};

class ParseRequest {
   public:
    class ParseException : public std::exception {
       public:
        ParseException(const char* message);
        const char* what() const throw();

       private:
        const char* _message;
    };
    ParseRequest();
    ~ParseRequest();
    void run(std::string& input);
    void showAll();  // use debug
                     //   const RequestLine& getRequestLine() const;  // not
                     //   use? const Fields& getFields() const; const Body&
                     //   getBody() const; const int getState() const; const
                     //   bool isCompleted() const; const bool hasError() const;
                     //   const int getErrorCode() const;

   private:
    ParseRequest(ParseRequest& other) {};
    ParseRequest& operator=(ParseRequest& other) { return *this; };
    void parseRequestLine();
    void parseFields();
    void parseBody();

    void validateRequestLine();
    void validateMethod(std::string& method);
    void validateUri(std::string& uri, std::string& path);
    void validateVersion(std::string& version);

    void splitRequestLine(std::string& line);
    void splitQuery(std::string& query);
    std::pair<std::string, std::vector<std::string>> splitFieldLine(
        std::string& line);

    RequestData _data;
};

bool isTraversalAttack(std::string& path);
bool caseInsensitiveCompare(const std::string& str1, const std::string& str2);
std::string trim(std::string& src, const std::string& sep);
bool hasCtlChar(std::string& str);
bool isUppStr(std::string& str);
bool caseInsensitiveCompare(const std::string& str1, const std::string& str2);
