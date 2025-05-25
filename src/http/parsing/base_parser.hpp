#pragma once

#include <string>
#include <limits>

#include "../http_namespace.hpp"
#include "../http_status.hpp"

namespace http {
class BaseParser {
 public:
    class ParseException : public std::exception {
     public:
        explicit ParseException(const char* message);
        const char* what() const throw();
     private:
        const char* _message;
    };
    enum ValidatePos {
        V_REQUEST_LINE = 0,
        V_FIELD = 1,
        V_BODY = 2,
        V_COMPLETED = 3,
    };
    enum ParseStatus {
        P_IN_PROGRESS = 0,
        P_NEED_MORE_DATA = 1,
        P_COMPLETED = 2,
        P_ERROR = 3
    };

    BaseParser() : _parseStatus(P_IN_PROGRESS), _clientMaxBodySize(-1) {}
    virtual ~BaseParser() {}

    BaseParser::ParseStatus run(const std::string& buf);
    ValidatePos getValidatePos() const { return _validatePos; }

 protected:
    BaseParser(const BaseParser& other);
    BaseParser& operator=(const BaseParser& other);

    virtual ParseStatus processRequestLine() = 0;
    virtual ParseStatus processFieldLine() = 0;
    virtual ParseStatus processBody() = 0;
    std::string* getBuf() { return &_buf; }
    void setBuf(std::string buf) { _buf = buf; }
    void setValidatePos(ValidatePos state) { _validatePos = state; }
    std::size_t findNewLinePos(std::string& buffer);
    std::size_t getLineEndLen(std::string& line, std::size_t lineEndPos);

 private:
    ValidatePos _validatePos;
    ParseStatus _parseStatus;
    std::string _buf;
    std::size_t _clientMaxBodySize;
};

}  // namespace http
