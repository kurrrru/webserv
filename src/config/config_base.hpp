// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>
#include <set>

#include "config_namespace.hpp"

namespace config {

class ConfigBase {
 public:
    ConfigBase();
    ConfigBase(const ConfigBase&);
    virtual ~ConfigBase();

    std::vector<std::string> allowed_methods;
    bool autoindex;
    std::vector<std::string> cgi_extensions;
    std::string cgi_pass;
    size_t client_max_body_size;
    std::vector<ErrorPage> error_pages;
    std::vector<std::string> indices;
    std::string root;
    std::string upload_store;

 private:
    ConfigBase& operator=(const ConfigBase&);
};

}  // namespace config
