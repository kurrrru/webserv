#pragma once

#include <map>
#include <string>
#include <vector>

#include "parse_request.hpp"

enum RequestState { REQUEST_LINE, HEADERS, BODY, COMPLETED };

struct RequestLine {
    std::string method;
    std::string uri;
    std::string version;
};

struct Fields {
    const std::string& get(const std::string& key) const;
    void set(const std::string& key, const std::string& value);

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
    void validateFields();
    void validateBody();
    bool CaseInsensitiveCompare(const std::string& str1,
                                const std::string& str2) const;
    RequestData _data;
};
