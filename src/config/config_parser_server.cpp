// Copyright 2025 Ideal Broccoli

#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_server.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool ConfigParser::parseServerBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config) {
    if (!validateBlockStart(tokens, pos, config::context::SERVER)) {
        return false;
    }
    if (!parseServerDirectives(tokens, pos, server_config)) {
        return false;
    }
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    server_config->setParent(_config.get());
    return true;
}

bool ConfigParser::parseServerDirectives(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config) {
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        (*pos)++;
        if (directive_name == config::context::LOCATION) {
            toolbox::SharedPtr<config::LocationConfig> location_config(new config::LocationConfig());
            (*pos)--;
            if (!parseLocationBlock(tokens, pos, server_config, location_config.get())) {
                return false;
            }
            server_config->locations.push_back(location_config);
        } else if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_SERVER)) {
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, NULL, server_config, NULL)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in server context.");
                return false;
            }
        } else {
            toolbox::logger::StepMark::error("Unknown or disallowed directive '" + directive_name + "' in server context.");
            return false;
        }
    }
    return true;
}

}  // namespace config
