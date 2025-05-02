// Copyright 2025 Ideal Broccoli

#include <vector>

#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_http.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool ConfigParser::parseHttpBlock(const std::vector<std::string>& tokens, size_t* pos) {
    if (!validateBlockStart(tokens, pos, config::context::HTTP)) {
        return false;
    }
    if (!parseHttpDirectives(tokens, pos, _config.get())) {
        return false;
    }
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
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
            config->servers.push_back(serverConfig);
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
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in HTTP context.");
                return false;
            }
        } else {
            toolbox::logger::StepMark::error("Unknown directive \"" + directive_name + "\"");
            return false;
        }
    }
    return true;
}

}  // namespace config
