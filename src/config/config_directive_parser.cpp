#include <algorithm>
#include <limits>
#include <string>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"
#include "config_util.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool expectSemicolon(const std::vector<std::string>& tokens, size_t* pos, const std::string directiveName) {
    if (*pos >= tokens.size() || tokens[*pos] != config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + directiveName + "\" directive");
    }
    (*pos)++;
    return true;
}

bool parseSize(const std::string& str, size_t* result) {
    if (!result || str.empty()) {
        toolbox::logger::StepMark::error("Unexpected Error");
        return false;
    }
    char unit = str[str.size() - 1];
    size_t len = str.size();
    size_t scale = 1;
    size_t max;
    switch (unit) {
        case 'K':
        case 'k':
            len--;
            max = std::numeric_limits<off_t>::max() / 1024;
            scale = 1024;
            break;
        case 'M':
        case 'm':
            len--;
            max = std::numeric_limits<off_t>::max() / (1024 * 1024);
            scale = 1024 * 1024;
            break;
        case 'G':
        case 'g':
            len--;
            max = std::numeric_limits<off_t>::max() / (1024 * 1024 * 1024);
            scale = 1024 * 1024 * 1024;
            break;
        default:
            if (!std::isdigit(unit)) {
                toolbox::logger::StepMark::error("Invalid unit in size value: " + str);
                return false;
            }
            max = std::numeric_limits<off_t>::max();
    }
    std::string num_str = str.substr(0, len);
    size_t value;
    if (!stringToSizeT(num_str, &value)) {
        return false;
    }
    if (value > max) {
        toolbox::logger::StepMark::error("Size value too large with scale applied: " + str);
        return false;
    }
    value *= scale;
    *result = value;
    return true;
}

void validateHost(const std::string& host, const std::string& fullValue) {
    struct addrinfo hints;
    struct addrinfo* result = NULL;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(host.c_str(), NULL, &hints, &result);
    if (status != 0) {
        throwConfigError("host not found in \"" + fullValue + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive: ");
    }
    if (result == NULL) {
        throwConfigError("host not found in \"" + fullValue + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive: ");
    }
    freeaddrinfo(result);
}

bool DirectiveParser::parseAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* methods) {
    if (!methods || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::ALLOWED_METHODS));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::ALLOWED_METHODS) + "\" directive");
    }
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string method = tokens[*pos];
        bool isValidMethod = false;
        for (size_t i = 0; i < config::method::ALLOWED_METHODS_COUNT; ++i) {
            if (isCaseInsensitiveIdentical(method, config::method::ALLOWED_METHODS[i])) {
                isValidMethod = true;
                break;
            }
        }
        if (!isValidMethod) {
            throwConfigError("Invalid value \"" +  method + "\" in \"" + std::string(config::directive::ALLOWED_METHODS) + "\" directive");
        }
        methods->push_back(method);
        (*pos)++;
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::ALLOWED_METHODS));
}

bool DirectiveParser::parseAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, bool* autoindex) {
    if (!autoindex || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::AUTOINDEX));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::AUTOINDEX) + "\" directive");
    }
    std::string value = tokens[(*pos)++];
    if (isCaseInsensitiveIdentical(value, config::directive::ON)) {
        *autoindex = true;
    } else if (isCaseInsensitiveIdentical(value, config::directive::OFF)) {
        *autoindex = false;
    } else {
        throwConfigError("Invalid value \"" +  value + "\" in \"" + std::string(config::directive::AUTOINDEX) + "\" directive");
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::AUTOINDEX));
}

bool DirectiveParser::parseCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* cgiExtensions) {
    if (!cgiExtensions || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CGI_EXTENSION));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::CGI_EXTENSION) + "\" directive");
    }
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string extension = tokens[(*pos)++];
        cgiExtensions->push_back(extension);
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::CGI_EXTENSION));
}

bool DirectiveParser::parseCgiPathDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgiPath) {
    if (!cgiPath || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CGI_PATH));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::CGI_PATH) + "\" directive");
    }
    std::string path = tokens[(*pos)++];
    *cgiPath = path;
    return expectSemicolon(tokens, pos, std::string(config::directive::CGI_PATH));
}

bool DirectiveParser::parseClientMaxBodySize(const std::vector<std::string>& tokens, size_t* pos, size_t* clientMaxBodySize) {
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CLIENT_MAX_BODY_SIZE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::CLIENT_MAX_BODY_SIZE) + "\" directive");
    }
    std::string sizeStr = tokens[(*pos)++];
    if (!parseSize(sizeStr, clientMaxBodySize)) {
        throwConfigError("\"" + std::string(config::directive::CLIENT_MAX_BODY_SIZE) + "\" directive invalid value");
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::CLIENT_MAX_BODY_SIZE));
}

bool DirectiveParser::parseErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ErrorPage>* errorPages) {
    if (!errorPages || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::ERROR_PAGE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::ERROR_PAGE) + "\" directive");
    }
    ErrorPage errorPage;
    std::vector<size_t> codes;
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string token = tokens[(*pos)];
        size_t codeValue;
        if (config::stringToSizeT(token, &codeValue)) {
            if (codeValue < config::directive::MIN_ERROR_PAGE_CODE || 
                codeValue > config::directive::MAX_ERROR_PAGE_CODE) {
                throwConfigError("value \"" + token + "\" must be between 300 and 599");
            }
            codes.push_back(codeValue);
            (*pos)++;
        } else {
            break;
        }
    }
    if (codes.empty() && tokens[*pos] != config::directive::SEMICOLON) {
        throwConfigError("invalid value \"" + std::string(tokens[*pos]) + "\"");
    } else if (codes.empty()) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::ERROR_PAGE) + "\"");
    }
    if (tokens[*pos][0] == config::directive::EQUAL) {
        errorPage.setOverwrite(true);
        if (*pos >= tokens.size() || tokens[*pos] == config::directive::SEMICOLON) {
            throwConfigError("invalid number of arguments in \"" + std::string(config::directive::ERROR_PAGE) + "\" directive after equal sign");
        }
        std::string newStatusCodeStr = tokens[*pos].substr(1);
        std::size_t newStatusCode;
        if (config::stringToSizeT(newStatusCodeStr, &newStatusCode)) {
            if (newStatusCode < config::directive::MIN_NEW_STATUS_CODE || 
                newStatusCode > config::directive::MAX_NEW_STATUS_CODE) {
                throwConfigError("invalid new status code \"" + newStatusCodeStr + "\" in \"" + std::string(config::directive::ERROR_PAGE) + "\" directive after equal sign");
            }
            errorPage.setNewStatusCode(static_cast<int>(newStatusCode));
        } else {
            throwConfigError("invalid new status code \"" + newStatusCodeStr + "\" in \"" + std::string(config::directive::ERROR_PAGE) + "\" directive after equal sign");
        }
        (*pos)++;
    }
    if (*pos >= tokens.size() || tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::ERROR_PAGE) + "\" directive after equal sign");
    }
    std::string path = tokens[(*pos)++];
    if (tokens[*pos] != config::directive::SEMICOLON) {
        throwConfigError("invalid value in \"" + tokens[*pos] + "\"");
    }
    for (size_t i = 0; i < codes.size(); ++i) {
        errorPage.addCode(codes[i]);
    }
    errorPage.setPath(path);
    errorPages->push_back(errorPage);
    return expectSemicolon(tokens, pos, std::string(config::directive::ERROR_PAGE));
}

bool DirectiveParser::parseIndexDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* indexFiles) {
    if (!indexFiles || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::INDEX));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::INDEX) + "\" directive");
    }
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string file = tokens[*pos];
        if (file.empty()) {
            throwConfigError("index \"" + tokens[*pos] + "\" in \"" + std::string(config::directive::INDEX) + "\" directive is invalid");
        }
        indexFiles->push_back(file);
        (*pos)++;
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::INDEX));
}

bool DirectiveParser::parseListenDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<Listen>* listen) {
    if (!listen || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::LISTEN));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::LISTEN) + "\" directive");
    }
    std::string listenValue = tokens[(*pos)++];
    Listen tmpListen;
    size_t port;
    size_t colonPos = listenValue.find(config::token::COLON);
    if (colonPos != std::string::npos) {
        std::string portStr = listenValue.substr(colonPos + 1);
        if (!config::stringToSizeT(portStr, &port))
            throwConfigError("invalid port in \"" + listenValue + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive");
        if (port < config::directive::MIN_PORT || port > config::directive::MAX_PORT) {
            throwConfigError("invalid port in \"" + portStr + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive");
        }
        tmpListen.setPort(port);
        std::string hostStr = listenValue.substr(0, colonPos);
        if (hostStr.empty()) {
            throwConfigError("no host in \"" + listenValue + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive");
        } else if (hostStr == config::directive::ASTERISK) {
            tmpListen.setIp(config::DEFAULT_IP);
        } else {
            validateHost(hostStr, listenValue);
            tmpListen.setIp(hostStr);
        }

    } else {
        if (config::stringToSizeT(listenValue, &port)) {
            if (port < config::directive::MIN_PORT || port > config::directive::MAX_PORT) {
                throwConfigError("invalid port in \"" + listenValue + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive");
            }
            tmpListen.setPort(port);
            tmpListen.setIp(config::DEFAULT_IP);
        } else {
            validateHost(listenValue, listenValue);
            tmpListen.setIp(listenValue);
        }
    }
    if (tokens[(*pos)] != config::directive::SEMICOLON) {
        if (tokens[(*pos)] == config::directive::LISTEN_DEFAULT_SERVER) {
            tmpListen.setDefaultServer(true);
            (*pos)++;
        } else {
            throwConfigError("invalid parameter \"" + tokens[(*pos)] + "\"" );
        }
    } else {
        tmpListen.setDefaultServer(false);
    }
    for (size_t i = 0; i < listen->size(); ++i) {
        if ((*listen)[i].getPort() == tmpListen.getPort() && (*listen)[i].getIp() == tmpListen.getIp()) {
            throwConfigError("duplicate ip port \"" + std::string(config::directive::LISTEN) + "\" directive");
        }
    }
    listen->push_back(tmpListen);

    return expectSemicolon(tokens, pos, std::string(config::directive::LISTEN));
}

bool DirectiveParser::parseReturnDirective(const std::vector<std::string>& tokens, size_t* pos, Return* returnValue) {
    if (!returnValue || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::RETURN));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::RETURN) + "\" directive");
    }
    std::string firstToken = tokens[(*pos)++];
    size_t code;
    if (!stringToSizeT(firstToken, &code)) {
        throwConfigError("Invalid return code: \"" + firstToken + "\"");
    }
    if (code > config::directive::MAX_RETURN_CODE) {
        throwConfigError("Invalid return code: \"" + firstToken + "\"");
    }
    returnValue->setStatusCode(code);
    returnValue->setHasReturnValue(true);
    if (tokens[*pos] == config::directive::SEMICOLON) {
        returnValue->setIsTextOrUrlSetting(false);
    } else {
        std::string secondToken = tokens[(*pos)++];
        returnValue->setTextOrUrl(secondToken);
        returnValue->setIsTextOrUrlSetting(true);
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::RETURN));
}

bool DirectiveParser::parseRootDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* root) {
    if (!root || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::ROOT));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::ROOT) + "\" directive");
    }
    std::string path = tokens[(*pos)++];
    *root = path;
    return expectSemicolon(tokens, pos, std::string(config::directive::ROOT));
}

bool DirectiveParser::parseServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ServerName>* serverNames) {
    if (!serverNames || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::SERVER_NAME));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::SERVER_NAME) + "\" directive");
    }
    std::vector<std::string> names;
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        names.push_back(tokens[*pos]);
        (*pos)++;
    }
    for (size_t i = 0; i < names.size(); ++i) {
        const std::string& name = names[i];
        ServerName serverName;
        if (name[0] == config::directive::ASTERISK[0]) {
            if (name.size() < 3 || name[1] != config::token::PERIOD[0]) {
                throwConfigError(std::string(config::directive::SERVER_NAME) + " \"" + name + "\" is invalid");
            }
            serverName.setType(ServerName::WILDCARD_START);
            serverName.setName(name);
        } else {
            serverName.setType(ServerName::EXACT);
            serverName.setName(name);
        }
        serverNames->push_back(serverName);
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::SERVER_NAME));
}

bool DirectiveParser::parseUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* uploadStore) {
    if (!uploadStore || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::UPLOAD_STORE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::UPLOAD_STORE) + "\" directive");
    }
    std::string path = tokens[(*pos)++];
    *uploadStore = path;
    return expectSemicolon(tokens, pos, std::string(config::directive::UPLOAD_STORE));
}

}  // namespace config
