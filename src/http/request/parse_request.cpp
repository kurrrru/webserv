#include "parse_request.hpp"

#include <iostream>

#include "http_namespace.hpp"

RequestParse::RequestParse() {};

RequestParse::~RequestParse() {};

RequestParse::ParseException::ParseException(const char* message)
    : _message(message) {}

const char* RequestParse::ParseException::what() const throw() {
    return _message;
}

void RequestParse::run(std::string& input) {
    _data.inputBuffer.append(input);
    parseRequestLine();
    // parseFields();
    // parseBody();
}

void RequestParse::parseRequestLine() {
    if (_data.requestState != START) {
        return;
    }
    if (_data.inputBuffer.find(http::CRLF) == std::string::npos) {
        return;
    }
    std::string line = trim(_data.inputBuffer, http::CRLF);
    std::cout << line << std::endl;
    splitRequestLine(line);
    validateRequestLine();
    std::cout << "method: " << _data.requestLine.method << std::endl;
    std::cout << "uri: " << _data.requestLine.uri << std::endl;
    std::cout << "version: " << _data.requestLine.version << std::endl;
    _data.requestState = REQUEST_LINE;
}

void RequestParse::splitRequestLine(std::string& line) {
    _data.requestLine.method = trim(line, http::SP);
    _data.requestLine.uri = trim(line, http::SP);
    _data.requestLine.version = trim(line, http::SP);
}

void RequestParse::validateRequestLine() {
    validateMethod(_data.requestLine.method);
    validateUri(_data.requestLine.uri);
    validateVersion(_data.requestLine.version);
}

void RequestParse::validateMethod(std::string& method) {
    if (method.empty()) {
        throw ParseException("Error: method doesn't exist");
    }
    if (hasCtlChar(method) || !isUppStr(method)) {
        throw ParseException("Error: method has control or lowercase character");
    }
    if (method != http::GET && method != http::POST && method != http::DELETE) {
        _data.errorCode = PARSE_INVALID_METHOD;
        throw ParseException("Error: method isn't supported");
    }
    return;
}

// need think normalizetion, query, fragment
void RequestParse::validateUri(std::string& uri) {
    if (uri.empty()) {
        throw ParseException("Error: uri doesn't exist");
    }
    if (*(uri.begin()) != '/' || hasCtlChar(uri)) {
        _data.errorCode = PARSE_INVALID_URI;
        throw ParseException("Error: missing '/' or uri has control character");
    }
    return;
}

void RequestParse::validateVersion(std::string& version) {
    if (version.empty()) {
        throw ParseException("Error: version doesn't exist");
    }
    if (hasCtlChar(version)) {
        throw ParseException("Error: version has control character");
    }
    if (version != http::HTTP_VERSION) {
        _data.errorCode = PARSE_INVALID_VERSION;
        throw ParseException("Error: version isn't supported");
    }
    return;
}

// void RequestParse::validateUri(std::string& uri) {
//     enum UriState { START, PATH, QUERY, FRAGMENT, END };
//     int state = START;
//     for (std::string::iterator it = uri.begin(); it != uri.end(); ++it) {
//         switch (state) {
//             case START:
//                 if (*it != '/') {
//                     throw ParseException("RequestParse: invalid uri START");
//                 }
//                 state = PATH;
//                 break;
//             case PATH:
//                 if (*it == '?') {
//                     state = QUERY;
//                 }
//                 break;
//             case QUERY:
//                 if (*it == '#') {
//                     state = FRAGMENT;
//                 }
//                 break;
//             case FRAGMENT:
//             default:
//                 throw ParseException("RequestParse: invalid uri default");
//         }
//     }
// }

/*
utils
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
