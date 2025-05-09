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

bool ConfigParser::parseServerBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config) {
    validateServerBlockStart(tokens, pos, config::context::SERVER);
    if (!parseServerDirectives(tokens, pos, server_config)) {
        return false;
    }
    validateBlockEnd(tokens, pos);
    server_config->setHttpParent(_config.get());
    return true;
}

bool ConfigParser::parseServerDirectives(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config) {
    std::map<std::string, bool> processed_directives;
    size_t location_counter = 0;
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        (*pos)++;
        if (directive_name == config::context::LOCATION) {
            location_counter++;
            toolbox::SharedPtr<config::LocationConfig> location_config(new config::LocationConfig());
            (*pos)--;
            if (!parseLocationBlock(tokens, pos, server_config, location_config.get())) {
                return false;
            }
            server_config->addLocation(location_config);
        } else if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_SERVER)) {
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
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, NULL, server_config, NULL)) {
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
    if (location_counter == 0) {
        toolbox::SharedPtr<config::LocationConfig> location_config(new config::LocationConfig());
        server_config->addLocation(location_config);
    }
    return true;
}

}  // namespace config
