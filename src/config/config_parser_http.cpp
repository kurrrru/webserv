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
    if (!validateServerBlocks()) {
        return false;
    }
    return true;
}

bool ConfigParser::validateServerBlocks() {
    if (_config->servers.empty()) {
        toolbox::logger::StepMark::error("No servers defined in HTTP context.");
        return false;
    }
    return true;
}

bool ConfigParser::parseHttpDirectives(const std::vector<std::string>& tokens, size_t* pos, config::HttpConfig* config) {
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
            if (config == NULL) {
                toolbox::logger::StepMark::error("Config is NULL in HTTP context.");
                return false;
            }
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, config, NULL, NULL)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in HTTP context.");
                return false;
            }
        } else {
            toolbox::logger::StepMark::error("Unknown directive \"" + directive_name + "\"");
            return false;
        }
    }
    toolbox::logger::StepMark::debug("Finished parsing directives in HTTP context.");
    return true;
}

}  // namespace config
