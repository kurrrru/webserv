#include <string>
#include <vector>

#include "config_base.hpp"

namespace config {

ErrorPage::ErrorPage() :
_codes(),
_path() {
}

ErrorPage::ErrorPage(const ErrorPage& other) :
_codes(other._codes),
_path(other._path) {
}

ErrorPage& ErrorPage::operator=(const ErrorPage& other) {
    if (this != &other) {
        _codes = other._codes;
        _path = other._path;
    }
    return *this;
}

ErrorPage::~ErrorPage() {
}

Listen::Listen() :
_port(DEFAULT_PORT),
_ip(DEFAULT_IP),
_defaultServer(false) {
}

Listen::Listen(const Listen& other) :
_port(other._port),
_ip(other._ip),
_defaultServer(other._defaultServer) {
}

Listen& Listen::operator=(const Listen& other) {
    if (this != &other) {
        _port = other._port;
        _ip = other._ip;
        _defaultServer = other._defaultServer;
    }
    return *this;
}

Listen::~Listen() {
}

ServerName::ServerName() :
_name(),
_type(EXACT) {
}

ServerName::ServerName(const ServerName& other) :
_name(other._name),
_type(other._type) {
}

ServerName& ServerName::operator=(const ServerName& other) {
    if (this != &other) {
        _name = other._name;
        _type = other._type;
    }
    return *this;
}

ServerName::~ServerName() {
}

Return::Return() :
_statusCode(),
_textOrUrl(),
_isTextOrUrlSetting(false),
_hasReturnValue(false) {
}

Return::Return(const Return& other) :
_statusCode(other._statusCode),
_textOrUrl(other._textOrUrl),
_isTextOrUrlSetting(other._isTextOrUrlSetting),
_hasReturnValue(other._hasReturnValue) {
}

Return& Return::operator=(const Return& other) {
    if (this != &other) {
        _statusCode = other._statusCode;
        _textOrUrl = other._textOrUrl;
        _isTextOrUrlSetting = other._isTextOrUrlSetting;
        _hasReturnValue = other._hasReturnValue;
    }
    return *this;
}

Return::~Return() {
}

ConfigBase::ConfigBase() :
_allowedMethods(),
_autoindex(DEFAULT_AUTOINDEX),
_cgiExtensions(),
_cgiPath(),
_clientMaxBodySize(DEFAULT_CLIENT_MAX_BODY_SIZE),
_errorPages(),
_indices(),
_root(DEFAULT_ROOT),
_uploadStore(DEFAULT_UPLOAD_STORE) {
}

ConfigBase::ConfigBase(const ConfigBase& other) :
_allowedMethods(other._allowedMethods),
_autoindex(other._autoindex),
_cgiExtensions(other._cgiExtensions),
_cgiPath(other._cgiPath),
_clientMaxBodySize(other._clientMaxBodySize),
_errorPages(other._errorPages),
_indices(other._indices),
_root(other._root),
_uploadStore(other._uploadStore) {
}

ConfigBase::~ConfigBase() {}

}  // namespace config
