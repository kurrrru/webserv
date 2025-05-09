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
_default_server(false) {
}

Listen::Listen(const Listen& other) :
_port(other._port),
_ip(other._ip),
_default_server(other._default_server) {
}

Listen& Listen::operator=(const Listen& other) {
    if (this != &other) {
        _port = other._port;
        _ip = other._ip;
        _default_server = other._default_server;
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
_status_code(),
_text_or_url(),
_is_text_or_url_setting(false),
_has_return_value(false) {
}

Return::Return(const Return& other) :
_status_code(other._status_code),
_text_or_url(other._text_or_url),
_is_text_or_url_setting(other._is_text_or_url_setting),
_has_return_value(other._has_return_value) {
}

Return& Return::operator=(const Return& other) {
    if (this != &other) {
        _status_code = other._status_code;
        _text_or_url = other._text_or_url;
        _is_text_or_url_setting = other._is_text_or_url_setting;
        _has_return_value = other._has_return_value;
    }
    return *this;
}

Return::~Return() {
}

ConfigBase::ConfigBase() :
_allowed_methods(),
_autoindex(DEFAULT_AUTOINDEX),
_cgi_extensions(),
_cgi_pass(DEFAULT_CGI_PATH),
_client_max_body_size(DEFAULT_CLIENT_MAX_BODY_SIZE),
_error_pages(),
_indices(),
_root(DEFAULT_ROOT),
_upload_store(DEFAULT_UPLOAD_STORE) {
}

ConfigBase::ConfigBase(const ConfigBase& other) :
_allowed_methods(other._allowed_methods),
_autoindex(other._autoindex),
_cgi_extensions(other._cgi_extensions),
_cgi_pass(other._cgi_pass),
_client_max_body_size(other._client_max_body_size),
_error_pages(other._error_pages),
_indices(other._indices),
_root(other._root),
_upload_store(other._upload_store) {
}

ConfigBase::~ConfigBase() {}

}  // namespace config
