// Copyright 2025 Ideal Broccoli

#include <algorithm>
#include <limits>
#include <string>

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"
#include "config_util.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool expectSemicolon(const std::vector<std::string>& tokens, size_t* pos, const std::string& directive_name) {
    if (*pos >= tokens.size() || tokens[*pos] != config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error("Expected semicolon after " + directive_name + " directive");
        return false;
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
    std::string num_str = str.std::string::substr(0, len);
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

bool DirectiveParser::parseAllowedMethodsDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* methods) {
    if (!methods || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::ALLOWED_METHODS));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::ALLOWED_METHODS) + " directive requires a value");
        (*pos)++;
        return false;
    }
    methods->clear();
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string method = tokens[*pos];
        bool is_valid_method = false;
        for (size_t i = 0; i < config::method::ALLOWED_METHODS_COUNT; ++i) {
            if (caseInsensitiveCompare(method, config::method::ALLOWED_METHODS[i])) {
                is_valid_method = true;
                break;
            }
        }
        if (!is_valid_method) {
            toolbox::logger::StepMark::error("invalid method: " + method);
            return false;
        }
        methods->push_back(method);
        (*pos)++;
    }
    return expectSemicolon(tokens, pos, config::directive::ALLOWED_METHODS);
}

bool DirectiveParser::parseAutoindexDirective(const std::vector<std::string>& tokens, size_t* pos, bool* autoindex) {
    if (!autoindex || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::AUTOINDEX));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::AUTOINDEX) + " directive requires a value");
        (*pos)++;
        return false;
    }
    std::string value = tokens[(*pos)++];
    if (caseInsensitiveCompare(value, config::directive::ON)) {
        *autoindex = true;
    } else if (caseInsensitiveCompare(value, config::directive::OFF)) {
        *autoindex = false;
    } else {
        toolbox::logger::StepMark::error("Invalid autoindex value: " + value);
        return false;
    }
    return expectSemicolon(tokens, pos, config::directive::AUTOINDEX);
}

bool DirectiveParser::parseCgiExtensionDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* cgi_extensions) {
    if (!cgi_extensions || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CGI_EXTENSION));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::CGI_EXTENSION) + " directive requires a value");
        (*pos)++;
        return false;
    }
    cgi_extensions->clear();
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string extension = tokens[(*pos)++];
        cgi_extensions->push_back(extension);
    }
    return expectSemicolon(tokens, pos, config::directive::CGI_EXTENSION);
}

bool DirectiveParser::parseCgiPassDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* cgi_pass) {
    if (!cgi_pass || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CGI_PASS));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::CGI_PASS) + " directive requires a value");
        (*pos)++;
        return false;
    }
    std::string path = tokens[(*pos)++];
    *cgi_pass = path;
    return expectSemicolon(tokens, pos, config::directive::CGI_PASS);
}

bool DirectiveParser::parseClientMaxBodySize(const std::vector<std::string>& tokens, size_t* pos, size_t* client_max_body_size) {
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::CLIENT_MAX_BODY_SIZE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::CLIENT_MAX_BODY_SIZE) + " directive requires a value");
        (*pos)++;
        return false;
    }
    std::string size_str = tokens[(*pos)++];
    if (!parseSize(size_str, client_max_body_size)) {
        return false;
    }
    return expectSemicolon(tokens, pos, config::directive::CLIENT_MAX_BODY_SIZE);
}

bool DirectiveParser::parseErrorPageDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ErrorPage>* error_pages) {    
    if (!error_pages || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::ERROR_PAGE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::ERROR_PAGE) + " directive requires a value");
        (*pos)++;
        return false;
    }
    std::vector<size_t> codes;
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string token = tokens[(*pos)];
        size_t code_value;
        if (config::stringToSizeT(token, &code_value)) {
            if (code_value < config::directive::MIN_ERROR_PAGE_CODE || 
                code_value > config::directive::MAX_ERROR_PAGE_CODE) {
                toolbox::logger::StepMark::error("value \"" + token + "\" must be between 300 and 599");
                return false;
            }
            codes.push_back(code_value);
            (*pos)++;
        } else {
            break;
        }
    }
    if (codes.empty()) {
        toolbox::logger::StepMark::error("invalid number of arguments in \"" + std::string(config::directive::ERROR_PAGE) + "\"");
        return false;
    }
    if (*pos >= tokens.size() || tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error("invalid number of arguments in \"" + std::string(config::directive::ERROR_PAGE) + "\"");
        return false;
    }
    std::string path = tokens[(*pos)++];
    if (tokens[*pos] != config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error("invalid value in \"" + tokens[*pos] + "\"");
        return false;
    }
    ErrorPage error_page;
    error_page.codes = codes;
    error_page.path = path;
    error_pages->push_back(error_page);
    return expectSemicolon(tokens, pos, config::directive::ERROR_PAGE);
}

bool DirectiveParser::parseIndexDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<std::string>* index_files) {
    if (!index_files || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::INDEX));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::INDEX) + " directive requires a value");
        (*pos)++;
        return false;
    }
    index_files->clear();
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        std::string file = tokens[*pos];
        if (file.empty()) {
            toolbox::logger::StepMark::error("index \"" + tokens[*pos] + "\" in \"" + std::string(config::directive::INDEX) + "\" directive is invalid");
            return false;
        }
        index_files->push_back(file);
        (*pos)++;
    }
    return expectSemicolon(tokens, pos, config::directive::INDEX);
}

// Pending
bool DirectiveParser::parseListenDirective(const std::vector<std::string>& tokens, size_t* pos, Listen* listen) {
    if (!listen || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::LISTEN));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::LISTEN) + " directive requires a value");
        (*pos)++;
        return false;
    }
    std::string port_str = tokens[(*pos)++];
    for (size_t i = 0; i < port_str.size(); ++i) {
        if (!std::isdigit(port_str[i])) {
            toolbox::logger::StepMark::error("Port must be a number: " + port_str);
            return false;
        }
    }
    int port_value = std::atoi(port_str.c_str());
    if (port_value <= 0 || port_value > 65535) {
        toolbox::logger::StepMark::error("Invalid port number: " + port_str + ". Must be between 1 and 65535");
        return false;
    }
    if (*pos >= tokens.size() || tokens[*pos] != config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error("Expected semicolon after listen directive");
        return false;
    }
    (*pos)++;
    listen->port = port_value;
    return true;
}

// Pending
bool DirectiveParser::parseReturnDirective(const std::vector<std::string>& tokens, size_t* pos, Return* return_value) {
    if (!return_value || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::RETURN));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::RETURN) + " directive requires a value");
        (*pos)++;
        return false;
    }
    std::string first_token = tokens[(*pos)++];
    bool first_is_code = true;
    for (size_t i = 0; i < first_token.size(); ++i) {
        if (!std::isdigit(first_token[i])) {
            first_is_code = false;
            break;
        }
    }
    if (first_is_code) {
        int code = std::atoi(first_token.c_str());
        if ((code < 300 || code > 308) && code != 201) {
            toolbox::logger::StepMark::warning("Return code " + first_token + " is not a standard redirect status code");
        }
        return_value->status_code = code;
        if (*pos >= tokens.size() || tokens[*pos] == config::directive::SEMICOLON) {
            toolbox::logger::StepMark::error("URL expected after status code in return directive");
            return false;
        }
        return_value->text_or_url = tokens[(*pos)++];
    } else {
        return_value->status_code = 302;
        return_value->text_or_url = first_token;
    }
    if (return_value->text_or_url.empty()) {
        toolbox::logger::StepMark::error("Return URL cannot be empty");
        return false;
    }
    if (*pos >= tokens.size() || tokens[*pos] != config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error("Expected semicolon after return directive");
        return false;
    }
    (*pos)++;
    return true;
}

// Pending
bool DirectiveParser::parseRootDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* root) {
    if (!root || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::SERVER_NAME));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::SERVER_NAME) + " directive requires a value");
        (*pos)++;
        return false;
    }
    std::string path = tokens[(*pos)++];
    if (path.empty()) {
        toolbox::logger::StepMark::error("Root path cannot be empty");
        return false;
    }
    if (path.find('\0') != std::string::npos) {
        toolbox::logger::StepMark::error("Root path contains null character");
        return false;
    }
    if (*pos >= tokens.size() || tokens[*pos] != config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error("Expected semicolon after root directive");
        return false;
    }
    (*pos)++;
    *root = path;
    toolbox::logger::StepMark::debug("Root path set to " + path);
    return true;
}

bool DirectiveParser::parseServerNameDirective(const std::vector<std::string>& tokens, size_t* pos, std::vector<ServerName>* server_names) {
    if (!server_names || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::SERVER_NAME));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::SERVER_NAME) + " directive requires a value");
        (*pos)++;
        return false;
    }
    server_names->clear();
    std::vector<std::string> names;
    while (*pos < tokens.size() && tokens[*pos] != config::directive::SEMICOLON) {
        names.push_back(tokens[*pos]);
        (*pos)++;
    }
    for (const std::string& name : names) {
        ServerName server_name;
        if (name.size() > 0 && name[0] == '*') {
            server_name.type = ServerName::WILDCARD_START;
            server_name.name = name.std::string::substr(1);
        } else if (name.size() > 0 && name[name.size() - 1] == '*') {
            server_name.type = ServerName::WILDCARD_END;
            server_name.name = name.std::string::substr(0, name.size() - 1);
        } else {
            server_name.type = ServerName::EXACT;
            server_name.name = name;
        }
        server_names->push_back(server_name);
    }
    return expectSemicolon(tokens, pos, config::directive::SERVER_NAME);
}

bool DirectiveParser::parseUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* upload_store) {
    if (!upload_store || *pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected Error :" + std::string(config::directive::UPLOAD_STORE));
        return false;
    }
    if (tokens[*pos] == config::directive::SEMICOLON) {
        toolbox::logger::StepMark::error(std::string(config::directive::UPLOAD_STORE) + " directive requires a value");
        (*pos)++;
        return false;
    }
    std::string path = tokens[(*pos)++];
    *upload_store = path;
    return expectSemicolon(tokens, pos, config::directive::UPLOAD_STORE);
}

}  // namespace config
