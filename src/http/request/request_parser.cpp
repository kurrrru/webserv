#include "request_parser.hpp"

#include "../http_namespace.hpp"
/*
Parser utils
*/

std::string trim(std::string& src, const std::string& sep) {
    std::size_t pos = src.find(sep);
    if (pos == std::string::npos && src.length() == 0) {
        return "";
    }
    std::string newStr = src.substr(0, pos);
    src.erase(0, pos + sep.length());
    return newStr;
}

bool hasCtlChar(std::string& str) {
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (std::iscntrl(*it)) {
            return true;
        }
    }
    return false;
}

bool isUppStr(std::string& str) {
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (!std::isupper(*it)) {
            return false;
        }
    }
    return true;
}

bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
    if (str1.length() != str2.length()) {
        return false;
    }
    for (std::size_t i = 0; i < str1.length(); ++i) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false;
        }
    }
    return true;
}

/*
Class method
*/

void RequestParser::run(const std::string& buf) {
    _buf.append(buf);
    parseRequestLine();
    parseFields();
    parseBody();
}

// class RequestParser {
//    public:
//     void run(const std::string& buf);
//     HTTPRequest get();

//    private:
//     HTTPRequest _request;
//     std::string _buf;
//                  _state

//     void parseRequestLine();
//     void parseFields();
//     void parseBody();

//     void validateMethod();
//     void validateURI();
//     void validateVersion();
// };

// enum ParseState { REQUEST_LINE, HEADERS, BODY, COMPLETED, ERROR };

void RequestParser::parseRequestLine() {
    if (_state != REQUEST_LINE || _buf.find(http::CRLF) == std::string::npos) {
        return;
    }
    std::string line = trim(_buf, http::CRLF);
    parseRequestLine();
    validateMethod();
    validateURI();
    validateVersion();
}
