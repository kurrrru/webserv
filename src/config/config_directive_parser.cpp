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

bool expectSemicolon(const std::vector<std::string>& tokens, size_t* pos, std::string directive_name) {
    if (*pos >= tokens.size() || tokens[*pos] != config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + directive_name + "\" directive");
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

void validateHost(const std::string& host, const std::string& full_value) {
    struct addrinfo hints;
    struct addrinfo* result = NULL;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(host.c_str(), NULL, &hints, &result);
    if (status != 0) {
        throwConfigError("host not found in \"" + full_value + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive: ");
    }
    if (result == NULL) {
        throwConfigError("host not found in \"" + full_value + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive: ");
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
        bool is_valid_method = false;
        for (size_t i = 0; i < config::method::ALLOWED_METHODS_COUNT; ++i) {
            if (isCaseInsensitiveIdentical(method, config::method::ALLOWED_METHODS[i])) {
                is_valid_method = true;
                break;
            }
        }
        if (!is_valid_method) {
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

bool DirectiveParser::parseCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* cgi_extensions) {
    if (!cgi_extensions || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CGI_EXTENSION));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::CGI_EXTENSION) + "\" directive");
    }
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string extension = tokens[(*pos)++];
        cgi_extensions->push_back(extension);
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::CGI_EXTENSION));
}

bool DirectiveParser::parseCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgi_pass) {
    if (!cgi_pass || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CGI_PASS));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::CGI_PASS) + "\" directive");
    }
    std::string path = tokens[(*pos)++];
    *cgi_pass = path;
    return expectSemicolon(tokens, pos, std::string(config::directive::CGI_PASS));
}

bool DirectiveParser::parseClientMaxBodySize(const std::vector<std::string>& tokens, size_t* pos, size_t* client_max_body_size) {
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CLIENT_MAX_BODY_SIZE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::CLIENT_MAX_BODY_SIZE) + "\" directive");
    }
    std::string size_str = tokens[(*pos)++];
    if (!parseSize(size_str, client_max_body_size)) {
        throwConfigError("\"" + std::string(config::directive::CLIENT_MAX_BODY_SIZE) + "\" directive invalid value");
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::CLIENT_MAX_BODY_SIZE));
}

bool DirectiveParser::parseErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ErrorPage>* error_pages) {
    if (!error_pages || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::ERROR_PAGE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::ERROR_PAGE) + "\" directive");
    }
    std::vector<size_t> codes;
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string token = tokens[(*pos)];
        size_t code_value;
        if (config::stringToSizeT(token, &code_value)) {
            if (code_value < config::directive::MIN_ERROR_PAGE_CODE || 
                code_value > config::directive::MAX_ERROR_PAGE_CODE) {
                throwConfigError("value \"" + token + "\" must be between 300 and 599");
            }
            codes.push_back(code_value);
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
    std::string path = tokens[(*pos)++];
    if (tokens[*pos] != config::directive::SEMICOLON) {
        throwConfigError("invalid value in \"" + tokens[*pos] + "\"");
    }
    ErrorPage error_page;
    for (size_t i = 0; i < codes.size(); ++i) {
        error_page.addCode(codes[i]);
    }
    error_page.setPath(path);
    error_pages->push_back(error_page);
    return expectSemicolon(tokens, pos, std::string(config::directive::ERROR_PAGE));
}

bool DirectiveParser::parseIndexDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* index_files) {
    if (!index_files || *pos >= tokens.size()) {
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
        index_files->push_back(file);
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
    std::string listen_value = tokens[(*pos)++];
    Listen tmp_listen;
    size_t port;
    size_t colon_pos = listen_value.find(config::token::COLON);
    if (colon_pos != std::string::npos) {
        std::string port_str = listen_value.substr(colon_pos + 1);
        if (!config::stringToSizeT(port_str, &port))
            throwConfigError("invalid port in \"" + listen_value + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive");
        if (port < config::directive::MIN_PORT || port > config::directive::MAX_PORT) {
            throwConfigError("invalid port in \"" + port_str + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive");
        }
        tmp_listen.setPort(port);
        std::string host_str = listen_value.substr(0, colon_pos);
        if (host_str.empty()) {
            throwConfigError("no host in \"" + listen_value + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive");
        } else if (host_str == config::directive::ASTERISK) {
            tmp_listen.setIp(config::DEFAULT_IP);
        } else {
            validateHost(host_str, listen_value);
            tmp_listen.setIp(host_str);
        }

    } else {
        if (config::stringToSizeT(listen_value, &port)) {
            if (port < config::directive::MIN_PORT || port > config::directive::MAX_PORT) {
                throwConfigError("invalid port in \"" + listen_value + "\" of the \"" + std::string(config::directive::LISTEN) + "\" directive");
            }
            tmp_listen.setPort(port);
            tmp_listen.setIp(config::DEFAULT_IP);
        } else {
            validateHost(listen_value, listen_value);
            tmp_listen.setIp(listen_value);
        }
    }
    if (tokens[(*pos)] != config::directive::SEMICOLON) {
        if (tokens[(*pos)] == config::directive::LISTEN_DEFAULT_SERVER) {
            tmp_listen.setDefaultServer(true);
            (*pos)++;
        } else {
            throwConfigError("invalid parameter \"" + tokens[(*pos)] + "\"" );
        }
    } else {
        tmp_listen.setDefaultServer(false);
    }
    for (size_t i = 0; i < listen->size(); ++i) {
        if ((*listen)[i].getPort() == tmp_listen.getPort() && (*listen)[i].getIp() == tmp_listen.getIp()) {
            throwConfigError("duplicate ip port \"" + std::string(config::directive::LISTEN) + "\" directive");
        }
    }
    listen->push_back(tmp_listen);

    return expectSemicolon(tokens, pos, std::string(config::directive::LISTEN));
}

bool DirectiveParser::parseReturnDirective(const std::vector<std::string>& tokens, size_t* pos, Return* return_value) {
    if (!return_value || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::RETURN));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::RETURN) + "\" directive");
    }
    std::string first_token = tokens[(*pos)++];
    size_t code;
    if (!stringToSizeT(first_token, &code)) {
        throwConfigError("Invalid return code: \"" + first_token + "\"");
    }
    if (code > config::directive::MAX_RETURN_CODE) {
        throwConfigError("Invalid return code: \"" + first_token + "\"");
    }
    return_value->setStatusCode(code);
    return_value->setHasReturnValue(true);
    if (tokens[*pos] == config::directive::SEMICOLON) {
        return_value->setIsTextOrUrlSetting(false);
    } else {
        std::string second_token = tokens[(*pos)++];
        return_value->setTextOrUrl(second_token);
        return_value->setIsTextOrUrlSetting(true);
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

bool DirectiveParser::parseServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ServerName>* server_names) {
    if (!server_names || *pos >= tokens.size()) {
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
        ServerName server_name;
        if (name[0] == config::directive::ASTERISK[0]) {
            if (name.size() < 3 || name[1] != config::token::PERIOD[0]) {
                throwConfigError(std::string(config::directive::SERVER_NAME) + " \"" + name + "\" is invalid");
            }
            server_name.setType(ServerName::WILDCARD_START);
            server_name.setName(name);
        } else {
            server_name.setType(ServerName::EXACT);
            server_name.setName(name);
        }
        server_names->push_back(server_name);
    }
    return expectSemicolon(tokens, pos, std::string(config::directive::SERVER_NAME));
}

bool DirectiveParser::parseUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* upload_store) {
    if (!upload_store || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::UPLOAD_STORE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::directive::UPLOAD_STORE) + "\" directive");
    }
    std::string path = tokens[(*pos)++];
    *upload_store = path;
    return expectSemicolon(tokens, pos, std::string(config::directive::UPLOAD_STORE));
}

}  // namespace config
