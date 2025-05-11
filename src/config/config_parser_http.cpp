#include <vector>

#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_http.hpp"
#include "config_util.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

void validateHttpBlockStart(const std::vector<std::string>& tokens, size_t* pos, const std::string& expectedDirective) {
    if (*pos >= tokens.size() || tokens[*pos] != expectedDirective) {
        if (isContextToken(tokens[*pos]) || isDirectiveToken(tokens[*pos])) {
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

bool ConfigParser::parseHttpBlock(const std::vector<std::string>& tokens, size_t* pos) {
    validateHttpBlockStart(tokens, pos, config::context::HTTP);
    if (!parseHttpDirectives(tokens, pos, _config.get())) {
        return false;
    }
    validateBlockEnd(tokens, pos);
    return true;
}

bool ConfigParser::parseHttpDirectives(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* config) {
    std::map<std::string, bool> processed_directives;
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        (*pos)++;
        if (directive_name == config::context::SERVER) {
            toolbox::SharedPtr<ServerConfig> serverConfig(new ServerConfig());
            (*pos)--;
            if (!parseServerBlock(tokens, pos, serverConfig.get())) {
                return false;
            }
            config->addServer(serverConfig);
        } else if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_HTTP)) {
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
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, config, NULL, NULL)) {
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

}  // namespace config
