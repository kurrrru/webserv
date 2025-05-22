#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_location.hpp"

#include "config_util.hpp"
#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"


namespace config {

void locationPathDuplicateCheck(const std::string& path, const config::ServerConfig& serverConfig) {
    const std::vector<toolbox::SharedPtr<config::LocationConfig> >& locations = serverConfig.getLocations();
    for (size_t i = 0; i < locations.size(); ++i) {
        if (path == locations[i]->getPath()) {
            throwConfigError("duplicate location \""  + path + "\"" );
        }
    }
}

void nestLocationPathDuplicateCheck(const std::string& path, const config::LocationConfig& parentConfig) {
    const std::vector<toolbox::SharedPtr<config::LocationConfig> >& locations = parentConfig.getLocations();
    for (size_t i = 0; i < locations.size(); ++i) {
        if (path == locations[i]->getPath()) {
            throwConfigError("duplicate location \""  + path + "\"");
        }
    }
}

void validateAndParseLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* serverConfig ,config::LocationConfig* locationConfig) {
    if (*pos >= tokens.size() || tokens[*pos] != config::context::LOCATION) {
        if (isContextToken(tokens[*pos])) {
            throwConfigError("\"" + std::string(tokens[*pos]) + "\" directive is not allowed here");
        } else {
            throwConfigError("unknown directive \"" + std::string(tokens[*pos]) + "\"");
        }
    }
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] == config::token::OPEN_BRACE) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::context::LOCATION) + "\" directive");
    }
    locationConfig->setPath(tokens[*pos]);
    locationPathDuplicateCheck(locationConfig->getPath(), *serverConfig);
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        throwConfigError("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \""+ std::string(config::token::CLOSE_BRACE) + "\"");
    }
    (*pos)++;
}

void validateAndParseNestedLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* parentConfig, config::LocationConfig* childConfig) {
    if (*pos >= tokens.size() || tokens[*pos] != config::context::LOCATION) {
        if (isContextToken(tokens[*pos])) {
            throwConfigError("\"" + std::string(tokens[*pos]) + "\" directive is not allowed here");
        } else {
            throwConfigError("unknown directive \"" + std::string(tokens[*pos]) + "\"");
        }
    }
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] == config::token::OPEN_BRACE) {
        throwConfigError("invalid number of arguments in \"" + std::string(config::context::LOCATION) + "\" directive");
    }
    childConfig->setPath(tokens[*pos]);
    nestLocationPathDuplicateCheck(childConfig->getPath(), *parentConfig);
    if (pathCmp(parentConfig->getPath(), childConfig->getPath()) != 0) {
        throwConfigError("location \"" + childConfig->getPath() +  "\" is outside location \"" + parentConfig->getPath() + "\"");
    }
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        throwConfigError("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \""+ std::string(config::token::CLOSE_BRACE) + "\"");
    }
    (*pos)++;
}

bool ConfigParser::parseLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* serverConfig, config::LocationConfig* locationConfig) {
    validateAndParseLocationBlockStart(tokens, pos, serverConfig, locationConfig);
    if (!parseLocationDirectives(tokens, pos, locationConfig)) {
        return false;
    }
    validateBlockEnd(tokens, pos);
    locationConfig->setServerParent(serverConfig);
    return true;
}

bool ConfigParser::parseNestedLocationBlock(const std::vector<std::string>& tokens,  size_t* pos,  config::LocationConfig* parentConfig, config::LocationConfig* childConfig) {
    validateAndParseNestedLocationBlockStart(tokens, pos, parentConfig, childConfig);
    if (!parseLocationDirectives(tokens, pos, childConfig)) {
        return false;
    }
    validateBlockEnd(tokens, pos);
    childConfig->setLocationParent(parentConfig);
    return true;
}

bool ConfigParser::parseLocationDirectives(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* locationConfig) {
    std::map<std::string, bool> processedDirectives;
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directiveName = tokens[*pos];
        (*pos)++;
        if (directiveName == config::context::LOCATION) {
            (*pos)--;
            if (!handleNestedLocationBlock(tokens, pos, locationConfig)) {
                return false;
            }
            continue;
        }
        if (_directiveParser.isDirectiveAllowedInContext(directiveName, config::CONTEXT_LOCATION)) {
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
            if (!_directiveParser.parseDirective(tokens, pos, directiveName, NULL, NULL, locationConfig)) {
                toolbox::logger::StepMark::error("Error parsing '" + directiveName + "' directive in location context.");
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

bool ConfigParser::handleNestedLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* parentLocation) {
    toolbox::SharedPtr<config::LocationConfig> nestedLocation(new config::LocationConfig());
    if (!parseNestedLocationBlock(tokens, pos, parentLocation, nestedLocation.get())) {
        return false;
    }
    parentLocation->addLocation(nestedLocation);
    return true;
}

}  // namespace config
