#include "parse_request.hpp"

#include <iostream>

/*
debug
*/

void ParseRequest::showAll() {
    std::cout << "----- headerLine -----" << std::endl;
    std::cout << "method: " << _data.requestLine.method << std::endl;
    std::cout << "uri: " << _data.requestLine.uri << std::endl;
    std::cout << "path: " << _data.requestLine.path << std::endl;
    std::cout << "query: " << _data.requestLine.query << std::endl;
    std::cout << "fragment: " << _data.requestLine.fragment << std::endl;
    std::cout << "version: " << _data.requestLine.version << std::endl;
    std::cout << "----- query map -----" << std::endl;
    for (std::map<std::string, std::string>::iterator it1 =
             _data.requestLine.queryMap.begin();
         it1 != _data.requestLine.queryMap.end(); ++it1) {
        std::cout << it1->first << ": " << it1->second << std::endl;
    }
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
    std::cout << "----- parse status ----\n"
              << _data.errorStatus.first << " " << _data.errorStatus.second
              << std::endl;
}

ParseRequest::ParseRequest() {};

ParseRequest::~ParseRequest() {};

ParseRequest::ParseException::ParseException(const char* message)
    : _message(message) {}

const char* ParseRequest::ParseException::what() const throw() {
    return _message;
}

void ParseRequest::run(std::string& input) {
    _data.inputBuffer.append(input);
    parseRequestLine();
    parseFields();
    parseBody();
}

/*
ParseRequestLine
*/

void ParseRequest::parseRequestLine() {
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

// METHOD URI[PATH QUERY FRAGMENT] VERSION
void ParseRequest::splitRequestLine(std::string& line) {
    _data.requestLine.method = trim(line, http::SP);
    _data.requestLine.uri = trim(line, http::SP);
    // std::size_t q_pos = _data.requestLine.uri.find(http::QUESTION);
    // std::size_t f_pos = _data.requestLine.uri.find(http::SHARP);
    // _data.requestLine.path = _data.requestLine.uri.substr(0, q_pos);
    // if (q_pos != std::string::npos && q_pos > f_pos) {
    //     _data.errorCode = PARSE_INVALID_URI;
    //     throw ParseException("Error: uri invalid order");
    // }
    // if (q_pos != std::string::npos) {
    //     std::size_t queryLength =
    //         (f_pos != std::string::npos) ? (f_pos - q_pos) :
    //         std::string::npos;
    //     _data.requestLine.query =
    //         _data.requestLine.uri.substr(q_pos, queryLength);
    //     splitQuery(_data.requestLine.query);
    // }
    // if (f_pos != std::string::npos) {
    //     _data.requestLine.fragment = _data.requestLine.uri.substr(f_pos);
    // }
    _data.requestLine.version = trim(line, http::SP);
}

void ParseRequest::splitQuery(std::string query) {
    query.erase(0, 1);
    std::cout << query << std::endl;
    while (!query.empty()) {
        std::size_t e_pos = query.find(http::EQUAL);
        std::size_t a_pos = query.find(http::AMPERSAND);
        if (a_pos != std::string::npos) {
            _data.requestLine.queryMap[query.substr(0, e_pos)] =
                query.substr(e_pos + 1, a_pos - e_pos - 1);
            query.erase(0, a_pos + 1);
        } else {
            _data.requestLine.queryMap[query.substr(0, e_pos)] =
                query.substr(e_pos + 1);
            query.clear();
        }
    }
}

void ParseRequest::validateRequestLine() {
    validateMethod(_data.requestLine.method);
    validateUri(_data.requestLine.uri, _data.requestLine.path);
    validateVersion(_data.requestLine.version);
}

void ParseRequest::validateMethod(std::string& method) {
    if (method.empty()) {
        _data.errorCode = PARSE_INVALID_METHOD;
        throw ParseException("Error: method doesn't exist");
    }
    if (hasCtlChar(method) || !isUppStr(method)) {
        _data.errorCode = PARSE_INVALID_METHOD;
        throw ParseException(
            "Error: method has control or lowercase character");
    }
    // need change
    if (method != http::GET && method != http::POST && method != http::DELETE) {
        _data.errorCode = PARSE_INVALID_METHOD;
        throw ParseException("Error: method isn't supported");
    }
    return;
}

// need think normalizetion, query, fragment
void ParseRequest::validateUri(std::string& uri, std::string& path) {
    if (uri.empty()) {
        _data.errorCode = PARSE_INVALID_URI;
        throw ParseException("Error: uri doesn't exist");
    }
    if (*(uri.begin()) != '/' || hasCtlChar(uri)) {
        _data.errorCode = PARSE_INVALID_URI;
        throw ParseException("Error: missing '/' or uri has control character");
    }
    if (isTraversalAttack(path)) {
        _data.errorCode = PARSE_INVALID_URI;
        throw ParseException("Error: warning! path is TraversalAttack");
    }
    return;
}

void ParseRequest::validateVersion(std::string& version) {
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

void ParseRequest::parseFields() {
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
            _data.inputBuffer =
                _data.inputBuffer.substr(2, _data.inputBuffer.length());
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
std::pair<std::string, std::vector<std::string>> ParseRequest::splitFieldLine(
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

// if, if, if contentLength > bodyLength??
void ParseRequest::parseBody() {
    if (_data.requestState != BODY) {
        return;
    }
    if (!_data.body.contentLength &&
        !_data.field.fields.find(http::CONTENT_LENGTH)->second.empty()) {
        _data.body.contentLength = std::atoi(
            (_data.field.fields.find(http::CONTENT_LENGTH)->second.begin())
                ->c_str());
    }
    if (_data.body.contentLength > _data.body.recvedLength) {
        _data.body.body.append(
            _data.inputBuffer.substr(0, _data.body.contentLength));
        _data.body.recvedLength += _data.inputBuffer.length();
    }
    if (_data.body.contentLength <= _data.body.recvedLength ||
        _data.inputBuffer.empty()) {
        _data.requestState = COMPLETED;
        _data.errorStatus = StatusCode::getStatusPair(OK);
    }
    _data.inputBuffer.clear();
}

/*
Field function
*/

void Fields::initField() {
    for (std::size_t i = 0; i < http::FIELD_SIZE; ++i) {
        fields.insert(
            std::make_pair(http::FIELDS[i], std::vector<std::string>()));
    }
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
local
*/

bool isTraversalAttack(std::string& path) {
    std::string normalizePath = path;
    std::size_t pos = 0;
    while ((pos = normalizePath.find("//", pos)) != std::string::npos) {
        normalizePath.replace(pos, 2, "/");
    }
    std::vector<std::string> splitPath;
    std::string parts;
    for (std::size_t i = 0; i < normalizePath.length(); ++i) {
        if (normalizePath[i] == '/' && !parts.empty()) {
            splitPath.push_back(parts);
            parts.clear();
        } else {
            parts += normalizePath[i];
        }
    }
    if (!parts.empty()) {
        splitPath.push_back(parts);
    }
    int level = 0;
    for (std::size_t i = 0; i < splitPath.size(); ++i) {
        if (splitPath[i] == "..") {
            --level;
            if (level < 0) {
                return true;
            }
        } else if (splitPath[i] != ".") {
            ++level;
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
