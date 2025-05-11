#include <string>

#include "config.hpp"
#include "config_http.hpp"

#include "../../toolbox/shared.hpp"

namespace config {

Config::Config() :
_token_count(0) {
}

Config::~Config() {
}

ConfigException::ConfigException(const std::string& message):
_message(message) {
}

ConfigException::~ConfigException() throw() {
}

const char* ConfigException::what() const throw() {
    return _message.c_str();
}

}  // namespace config
