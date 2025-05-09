#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_location.hpp"

#include "config_util.hpp"
#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"


namespace config {

void validateAndParseLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* location_config) {
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
    location_config->setPath(tokens[*pos]);
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        throwConfigError("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \""+ std::string(config::token::CLOSE_BRACE) + "\"");
    }
    (*pos)++;
}

void validateAndParseNestedLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* parent_config, config::LocationConfig* child_config) {
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
    child_config->setPath(tokens[*pos]);
    if (pathCmp(parent_config->getPath(), child_config->getPath()) != 0) {
        throwConfigError("location \"" + child_config->getPath() +  "\" is outside location \"" + parent_config->getPath() + "\"");
    }
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        throwConfigError("unexpected end of file, expecting \"" + std::string(config::token::SEMICOLON) + "\" or \""+ std::string(config::token::CLOSE_BRACE) + "\"");
    }
    (*pos)++;
}

bool ConfigParser::parseLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config, config::LocationConfig* location_config) {
    validateAndParseLocationBlockStart(tokens, pos, location_config);
    if (!parseLocationDirectives(tokens, pos, location_config)) {
        return false;
    }
    validateBlockEnd(tokens, pos);
    location_config->setServerParent(server_config);
    return true;
}

bool ConfigParser::parseNestedLocationBlock(const std::vector<std::string>& tokens,  size_t* pos,  config::LocationConfig* parent_config, config::LocationConfig* child_config) {
    validateAndParseNestedLocationBlockStart(tokens, pos, parent_config, child_config);
    if (!parseLocationDirectives(tokens, pos, child_config)) {
        return false;
    }
    validateBlockEnd(tokens, pos);
    child_config->setLocationParent(parent_config);
    return true;
}

bool ConfigParser::parseLocationDirectives(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* location_config) {
    std::map<std::string, bool> processed_directives;
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        (*pos)++;
        if (directive_name == config::context::LOCATION) {
            (*pos)--;
            if (!handleNestedLocationBlock(tokens, pos, location_config)) {
                return false;
            }
            continue;
        }
        if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_LOCATION)) {
            if (processed_directives.find(directive_name) != processed_directives.end()) {
                bool should_skip = false;
                if (!_directiveParser.handleDuplicateDirective(directive_name, tokens, pos, &should_skip)) {
                    return false;
                }
                if (should_skip) {
                    continue;
                }
            }
            processed_directives[directive_name] = true;
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, NULL, NULL, location_config)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in location context.");
                return false;
            }
        } else {
            if (isContextToken(directive_name)) {
                throwConfigError("\"" + directive_name + "\" directive is not allowed here");
            } else {
                throwConfigError("Unknown directive \"" + directive_name + "\"");
            }
        }
    }
    return true;
}

bool ConfigParser::handleNestedLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* parent_location) {
    toolbox::SharedPtr<config::LocationConfig> nested_location(new config::LocationConfig());
    if (!parseNestedLocationBlock(tokens, pos, parent_location, nested_location.get())) {
        return false;
    }
    parent_location->addLocation(nested_location);
    return true;
}

}  // namespace config
