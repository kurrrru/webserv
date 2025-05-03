#pragma once

#include <string>

namespace http {

class BaseParser {
    public:
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
    std::string _buf;
    ParseState _validatePos;

    virtual void parseFieldLine() = 0;
    virtual void parseBody() = 0;

    std::size_t findNewLinePos(std::string& buffer);
    std::size_t getLineEndLen(std::string& line, std::size_t lineEndPos);
};

}
