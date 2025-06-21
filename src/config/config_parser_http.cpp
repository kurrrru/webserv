#include <vector>

#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_http.hpp"
#include "config_util.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

void validateHttpBlockStart(const std::vector<std::string>& tokens, std::size_t* pos, const std::string& expectedDirective) {
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

bool ConfigParser::parseHttpBlock(const std::vector<std::string>& tokens, std::size_t* pos) {
    validateHttpBlockStart(tokens, pos, config::context::HTTP);
    if (!parseHttpDirectives(tokens, pos, _config.get())) {
        return false;
    }
    validateBlockEnd(tokens, pos);
    return true;
}

bool ConfigParser::parseHttpDirectives(const std::vector<std::string>& tokens, std::size_t* pos, config::HttpConfig* config) {
    std::map<std::string, bool> processedDirectives;
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directiveName = tokens[*pos];
        (*pos)++;
        if (directiveName == config::context::SERVER) {
            toolbox::SharedPtr<ServerConfig> serverConfig(new ServerConfig());
            (*pos)--;
            if (!parseServerBlock(tokens, pos, serverConfig.get())) {
                return false;
            }
            config->addServer(serverConfig);
        } else if (_directiveParser.isDirectiveAllowedInContext(directiveName, config::CONTEXT_HTTP)) {
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
            if (!_directiveParser.parseDirective(tokens, pos, directiveName, config, NULL, NULL)) {
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
