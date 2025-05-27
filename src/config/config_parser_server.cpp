#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_server.hpp"
#include "config_util.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

void validateServerBlockStart(const std::vector<std::string>& tokens, size_t* pos, const std::string& expectedDirective) {
    if (*pos >= tokens.size() || tokens[*pos] != expectedDirective) {
        if (isContextToken(tokens[*pos])) {
            throwConfigError("\"" + std::string(tokens[*pos]) + "\" directive is not allowed here");
        } else {
            throwConfigError("unknown directive \"" + std::string(tokens[*pos]) + "\"");
        }
    }
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        throwConfigError("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \""+ std::string(config::token::CLOSE_BRACE) + "\"");
    }
    (*pos)++;
}

bool ConfigParser::parseServerBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* serverConfig) {
    validateServerBlockStart(tokens, pos, config::context::SERVER);
    if (!parseServerDirectives(tokens, pos, serverConfig)) {
        return false;
    }
    validateBlockEnd(tokens, pos);
    serverConfig->setHttpParent(_config.get());
    return true;
}

bool ConfigParser::parseServerDirectives(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* serverConfig) {
    std::map<std::string, bool> processedDirectives;
    size_t locationCounter = 0;
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directiveName = tokens[*pos];
        (*pos)++;
        if (directiveName == config::context::LOCATION) {
            locationCounter++;
            toolbox::SharedPtr<config::LocationConfig> locationConfig(new config::LocationConfig());
            (*pos)--;
            if (!parseLocationBlock(tokens, pos, serverConfig, locationConfig.get())) {
                return false;
            }
            serverConfig->addLocation(locationConfig);
        } else if (_directiveParser.isDirectiveAllowedInContext(directiveName, config::CONTEXT_SERVER)) {
            if (processedDirectives.find(directiveName) != processedDirectives.end()) {
                bool shouldSkip = false;
                if (!_directiveParser.handleDuplicateDirective(directiveName, tokens, pos, &shouldSkip)) {
                    return false;
                }
                if (shouldSkip) {
                    continue;
                }
            }
            processedDirectives[directiveName] = true;
            if (!_directiveParser.parseDirective(tokens, pos, directiveName, NULL, serverConfig, NULL)) {
                return false;
            }
        } else {
            if (isContextToken(directiveName)) {
                throwConfigError("\"" + directiveName + "\" directive is not allowed here");
            } else {
                throwConfigError("Unknown directive \"" + directiveName + "\"");
            }
        }
    }
    return true;
}

}  // namespace config
