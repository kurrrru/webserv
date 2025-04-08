#include "parse_request.hpp"

#include <iostream>

#include "http_namespace.hpp"

/*
debug
*/

void printMap(std::map<std::string, std::vector<std::string>> field) {

}

void RequestParse::showAll() {
    std::cout << "----- headerLine -----" << std::endl;
    std::cout << "method: " << _data.requestLine.method << std::endl;
    std::cout << "uri: " << _data.requestLine.uri << std::endl;
    std::cout << "version: " << _data.requestLine.version << std::endl;
    std::cout << "----- parsed fields -----" << std::endl;
    for (std::map<std::string, std::vector<std::string>>::iterator it1 =
        _data.field.fields.begin();
        it1 != _data.field.fields.end(); ++it1) {
            if (it1->second.empty()) {
                continue;
            }
            std::cout << it1->first << " ->> ";
            for (std::vector<std::string>::iterator it2 = it1->second.begin();
            it2 != it1->second.end(); ++it2) {
                std::cout << *it2;
            }
            std::cout << std::endl;
        }
    std::cout << "----- recv body -----\n" << _data.body.body << std::endl; 
}

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
    parseFields();
    parseBody();
}

/*
ParseRequestLine
*/

void RequestParse::parseRequestLine() {
    if (_data.requestState != REQUEST_LINE) {
        return;
    }
    if (_data.inputBuffer.find(http::CRLF) == std::string::npos) {
        return;
    }
    std::string line = trim(_data.inputBuffer, http::CRLF);
    splitRequestLine(line);
    validateRequestLine();
    _data.requestState = HEADERS;
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
        _data.errorCode = PARSE_INVALID_METHOD;
        throw ParseException("Error: method doesn't exist");
    }
    if (hasCtlChar(method) || !isUppStr(method)) {
        _data.errorCode = PARSE_INVALID_METHOD;
        throw ParseException(
            "Error: method has control or lowercase character");
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
        _data.errorCode = PARSE_INVALID_URI;
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
        _data.errorCode = PARSE_INVALID_VERSION;
        throw ParseException("Error: version doesn't exist");
    }
    if (hasCtlChar(version)) {
        _data.errorCode = PARSE_INVALID_VERSION;
        throw ParseException("Error: version has control character");
    }
    if (version != http::HTTP_VERSION) {
        _data.errorCode = PARSE_INVALID_VERSION;
        throw ParseException("Error: version isn't supported");
    }
    return;
}

/*
ParseFields
*/

void RequestParse::parseFields() {
    if (_data.requestState != HEADERS) {
        return;
    }
    if (_data.inputBuffer.find(http::CRLF) == std::string::npos) {
        return;
    }
    if (_data.field.fields.empty()) {
        _data.field.initField();
    }
    for (;;) {
        if (_data.inputBuffer.find(http::CRLF) == 0) {
            _data.requestState = BODY;
            _data.inputBuffer = _data.inputBuffer.substr(2, _data.inputBuffer.length());
            return;
        }
        if (_data.inputBuffer.find(http::CRLF) == std::string::npos) {
            return;
        }
        std::string line = trim(_data.inputBuffer, http::CRLF);
        std::pair<std::string, std::vector<std::string>> pair =
            splitFieldLine(line);
        _data.field.set(pair);
    }
}

// valid line only
// need split line
std::pair<std::string, std::vector<std::string>> RequestParse::splitFieldLine(
    std::string& line) {
    std::size_t pos = line.find_first_of(':');
    if (pos == std::string::npos) {
        throw ParseException("Error: invalid field line");
    }
    std::pair<std::string, std::vector<std::string>> pair;
    pair.first = trim(line, ": ");
    pair.second.push_back(line);
    return pair;
}

/*
ParseBody
*/

//if, if, if contentLength > bodyLength??
void RequestParse::parseBody() {
    if (_data.requestState != BODY) {
        return;
    }
    if (!_data.body.contentLength && !_data.field.fields.find(http::ContentLength)->second.empty()) {
        _data.body.contentLength = std::atoi((_data.field.fields.find(http::ContentLength)->second.begin())->c_str());
    }
    if (_data.body.contentLength > _data.body.recvedLength) {
        _data.body.body.append(_data.inputBuffer.substr(0, _data.body.contentLength));
        _data.body.recvedLength += _data.inputBuffer.length();
    }
    if (_data.body.contentLength <= _data.body.recvedLength || _data.inputBuffer.empty()) {
        _data.requestState = COMPLETED;
        return;
    }
    _data.inputBuffer.clear();
}

/*
Field function
*/

void Fields::initField() {
    fields.insert(std::make_pair(http::Host, std::vector<std::string>()));
    fields.insert(
        std::make_pair(http::ContentType, std::vector<std::string>()));
    fields.insert(
        std::make_pair(http::ContentLength, std::vector<std::string>()));
    fields.insert(std::make_pair(http::UserAgent, std::vector<std::string>()));
    fields.insert(std::make_pair(http::Connection, std::vector<std::string>()));
    fields.insert(std::make_pair(http::Accept, std::vector<std::string>()));
}

bool Fields::set(std::pair<std::string, std::vector<std::string>>& pair) {
    for (std::map<std::string, std::vector<std::string>>::iterator m_it =
             fields.begin();
         m_it != fields.end(); ++m_it) {
        if (caseInsensitiveCompare(m_it->first, pair.first)) {
            for (std::vector<std::string>::iterator v_it = pair.second.begin();
                 v_it != pair.second.end(); v_it++) {
                m_it->second.push_back(*v_it);
            }
            return true;
        }
    }
    return false;
}

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
