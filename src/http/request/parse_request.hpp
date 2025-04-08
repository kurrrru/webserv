#pragma once

#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "http_namespace.hpp"
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
    std::string version;
};

struct Fields {
    void initField();
    const std::string& get(const std::string& key) const;
    bool set(std::pair<std::string, std::vector<std::string>>& pair);

    std::map<std::string, std::vector<std::string> > fields;
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
    RequestLine requestLine;
    Fields field;
    Body body;
    std::string inputBuffer;
};

class RequestParse {
   public:
    class ParseException : public std::exception {
       public:
        ParseException(const char* message);
        const char* what() const throw();

       private:
        const char* _message;
    };
    RequestParse();
    ~RequestParse();
    void run(std::string& input);
    const RequestLine& getRequestLine() const;
    const Fields& getFields() const;
    const Body& getBody() const;
    const int getState() const;
    const bool isCompleted() const;
    const bool hasError() const;
    const int getErrorCode() const;
    void showAll();

   private:
    RequestParse(RequestParse& other) {};
    RequestParse& operator=(RequestParse& other) { return *this; };
    void parseRequestLine();
    void parseFields();
    void parseBody();
    std::string readRequestLine();
    std::pair<std::string, std::vector<std::string> > readFields();
    std::string readBody();
    void validateRequestLine();
    void validateMethod(std::string& method);
    void validateUri(std::string& uri);
    void validateVersion(std::string& version);
    void validateFields();
    void validateBody();
    void splitRequestLine(std::string& line);
    std::pair<std::string, std::vector<std::string> > splitFieldLine(
        std::string& line);

    RequestData _data;
    };

    bool caseInsensitiveCompare(const std::string& str1,
                                const std::string& str2);
    std::string trim(std::string& src, const std::string& sep);
    bool hasCtlChar(std::string& str);
    bool isUppStr(std::string& str);
    bool caseInsensitiveCompare(const std::string& str1, const std::string& str2);
