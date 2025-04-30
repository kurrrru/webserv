// Copyright 2025 Ideal Broccoli

#include "config_parser.hpp"
#include "config_namespace.hpp"
#include "config_location.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace config {

bool ConfigParser::parseLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::ServerConfig* server_config, config::LocationConfig* location_config) {
    if (!validateAndParseLocationBlockStart(tokens, pos, location_config)) {
        return false;
    }
    if (!parseLocationDirectives(tokens, pos, location_config)) {
        return false;
    }
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    location_config->setParent(server_config);
    return true;
}

bool ConfigParser::parseNestedLocationBlock(const std::vector<std::string>& tokens,  size_t* pos,  config::LocationConfig* parent_location, config::LocationConfig* location_config) {
    if (!validateAndParseLocationBlockStart(tokens, pos, location_config)) {
        return false;
    }
    if (!parseLocationDirectives(tokens, pos, location_config)) {
        return false;
    }
    if (!validateBlockEnd(tokens, pos)) {
        return false;
    }
    location_config->setParent(parent_location);
    return true;
}

bool ConfigParser::validateAndParseLocationBlockStart(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* location_config) {
    if (*pos >= tokens.size() || tokens[*pos] != config::context::LOCATION) {
        toolbox::logger::StepMark::error("Expected 'location' directive.");
        return false;
    }
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] == config::token::OPEN_BRACE) {
        toolbox::logger::StepMark::error("Expected path after 'location' directive.");
        return false;
    }
    location_config->path = tokens[*pos];
    toolbox::logger::StepMark::debug("Location path: " + location_config->path);
    (*pos)++;
    if (*pos >= tokens.size() || tokens[*pos] != config::token::OPEN_BRACE) {
        toolbox::logger::StepMark::error("Expected '{' after location path.");
        return false;
    }
    (*pos)++;
    return true;
}

bool ConfigParser::parseLocationDirectives(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* location_config) {
    while (*pos < tokens.size() && tokens[*pos] != config::token::CLOSE_BRACE) {
        std::string directive_name = tokens[*pos];
        if (directive_name == config::context::LOCATION) {
            if (!handleNestedLocationBlock(tokens, pos, location_config)) {
                return false;
            }
            continue;
        }
        (*pos)++;
        if (_directiveParser.isDirectiveAllowedInContext(directive_name, config::CONTEXT_LOCATION)) {
            if (!_directiveParser.parseDirective(tokens, pos, directive_name, NULL, NULL, location_config)) {
                toolbox::logger::StepMark::error("Error parsing '" + directive_name + "' directive in location context.");
                return false;
            }
        } else {
            toolbox::logger::StepMark::error("Unknown or disallowed directive '" + directive_name + "' in location context.");
            return false;
        }
    }
    return true;
}

bool ConfigParser::handleNestedLocationBlock(const std::vector<std::string>& tokens, size_t* pos, config::LocationConfig* parent_location) {
    toolbox::SharedPtr<config::LocationConfig> nested_location(new config::LocationConfig());
    if (!parseNestedLocationBlock(tokens, pos, parent_location, nested_location.get())) {
        return false;
    }
    parent_location->addLocation(*nested_location.get());
    toolbox::logger::StepMark::debug("Added nested location: " + nested_location->path + " to parent: " + parent_location->path);
    return true;
}

}  // namespace config
