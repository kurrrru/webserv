// Copyright 2025 Ideal Broccoli

#include <string>
#include <vector>

#include "config_base.hpp"

namespace config {

ConfigBase::ConfigBase() :
allowed_methods(),
autoindex(DEFAULT_AUTOINDEX),
cgi_extensions(),
cgi_pass(DEFAULT_CGI_PATH),
client_max_body_size(DEFAULT_CLIENT_MAX_BODY_SIZE),
error_pages(),
indices(),
root(DEFAULT_ROOT),
upload_store(DEFAULT_UPLOAD_STORE) {
}

ConfigBase::ConfigBase(const ConfigBase& other) :
allowed_methods(other.allowed_methods),
autoindex(other.autoindex),
cgi_extensions(other.cgi_extensions),
cgi_pass(other.cgi_pass),
client_max_body_size(other.client_max_body_size),
error_pages(other.error_pages),
indices(other.indices),
root(other.root),
upload_store(other.upload_store) {
}

ConfigBase::~ConfigBase() {}

}  // namespace config
