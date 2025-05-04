#pragma once

#include <string>

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
    enum ParseState {
        REQUEST_LINE = 0,
        HEADERS = 1,
        BODY = 2,
        COMPLETED = 3,
        ERROR = 4
    };

    BaseParser() {}
    virtual ~BaseParser() {}
    virtual void run(const std::string& buf) = 0;

 protected:
    virtual void processFieldLine() = 0;
    virtual void processBody() = 0;

    std::size_t findNewLinePos(std::string& buffer);
    std::size_t getLineEndLen(std::string& line, std::size_t lineEndPos);

    std::string _buf;
    ParseState _validatePos;
};

}  // namespace http
