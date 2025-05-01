#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "../../src/config/config.hpp"
#include "../../src/config/config_server.hpp"
#include "../../src/config/config_location.hpp"
#include "../../src/config/config_namespace.hpp"
#include "../../src/config/config_parser.hpp"
#include "../../toolbox/shared.hpp"
#include "../../toolbox/string.hpp"
#include "../../toolbox/stepmark.hpp"

void printSettings(const std::string& prefix, const config::ConfigBase& conf) {
    std::cout << prefix << "allowed_methods: ";
    for (size_t i = 0; i < conf.allowed_methods.size(); ++i) {
        std::cout << conf.allowed_methods[i];
        if (i < conf.allowed_methods.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << prefix << "autoindex: " << (conf.autoindex ? "on" : "off") << std::endl;
    std::cout << prefix << "cgi_extensions: ";
    for (size_t i = 0; i < conf.cgi_extensions.size(); ++i) {
        std::cout << conf.cgi_extensions[i];
        if (i < conf.cgi_extensions.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << prefix << "cgi_pass: " << conf.cgi_pass << std::endl;
    std::cout << prefix << "client_max_body_size: " << conf.client_max_body_size << std::endl;
    std::cout << prefix << "error_pages: ";
    for (size_t i = 0; i < conf.error_pages.size(); ++i) {
        std::cout << "[";
        for (size_t j = 0; j < conf.error_pages[i].codes.size(); ++j) {
            std::cout << conf.error_pages[i].codes[j];
            if (j < conf.error_pages[i].codes.size() - 1) std::cout << ", ";
        }
        std::cout << "] -> " << conf.error_pages[i].path;
        if (i < conf.error_pages.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << prefix << "indices: ";
    for (size_t i = 0; i < conf.indices.size(); ++i) {
        std::cout << conf.indices[i];
        if (i < conf.indices.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << prefix << "root: " << conf.root << std::endl;
    std::cout << prefix << "upload_store: " << conf.upload_store << std::endl;

    std::cout << std::endl << std::endl;
}

int main() {
    toolbox::logger::StepMark::setLogFile("config_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    try {
        std::string config_file = "../../conf/default.conf";
        std::cout << "===== parse start =====" << std::endl << std::endl;
        toolbox::SharedPtr<config::Config> config;
        try {
            config = config::ConfigParser::parseFile(config_file, false);
            std::cout << "parse success" << std::endl << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "parse fail : " << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        const toolbox::SharedPtr<config::HttpConfig>& http = config->getHttpConfig();
        std::cout << "===== HTTP =====" << std::endl;
        printSettings("", *http);
        for (size_t i = 0; i < http->servers.size(); ++i) {
            std::cout << "===== SERVER =====" << std::endl;
            std::cout << "server #" << (i + 1) << std::endl;
            std::cout << "listen: ";
            for (size_t listen_idx = 0; listen_idx < http->servers[i]->listens.size(); ++listen_idx) {
                const config::Listen& listen_config = http->servers[i]->listens[listen_idx];
                std::cout << listen_config.ip << ":" << listen_config.port;
                if (listen_config.default_server) {
                    std::cout << " (default)";
                }
                if (listen_idx < http->servers[i]->listens.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
            std::cout << "server_name: ";
            for (size_t j = 0; j < http->servers[i]->server_names.size(); ++j) {
                const config::ServerName& server_name = http->servers[i]->server_names[j];
                if (server_name.type == config::ServerName::WILDCARD_START) {
                    std::cout << "*" << server_name.name;
                } else if (server_name.type == config::ServerName::WILDCARD_END) {
                    std::cout << server_name.name << "*";
                } else {
                    std::cout << server_name.name;
                }
                if (j < http->servers[i]->server_names.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
            if (http->servers[i]->return_value.has_return_value) {
                std::cout << "return code: " << http->servers[i]->return_value.status_code << std::endl;
                if (http->servers[i]->return_value.is_text_or_url_setting) {
                    std::cout << "return text_or_url: " << http->servers[i]->return_value.text_or_url << std::endl;
                }
            }
            printSettings("", *http->servers[i]);
            for (size_t j = 0; j < http->servers[i]->locations.size(); ++j) {
                std::cout << "===== LOCATION =====" << std::endl;
                std::cout << "location #" << (j + 1) << ":" << std::endl;
                std::cout << "path: " << http->servers[i]->locations[j]->path << std::endl;
                if (http->servers[i]->return_value.has_return_value) {
                    std::cout << "return code: " << http->servers[i]->return_value.status_code << std::endl;
                    if (http->servers[i]->return_value.is_text_or_url_setting) {
                        std::cout << "return text_or_url: " << http->servers[i]->return_value.text_or_url << std::endl;
                    }
                }
                printSettings("", *http->servers[i]->locations[j]);
                for (size_t k = 0; k < http->servers[i]->locations[j]->locations.size(); ++k) {
                    std::cout << "===== NESTEDLOCATION =====" << std::endl;
                    std::cout << "nestedlocation #" << (k + 1) << ": " << std::endl;
                    std::cout << "path: " << http->servers[i]->locations[j]->locations[k]->path << std::endl;
                    printSettings("", *http->servers[i]->locations[j]->locations[k]);
                }
            }
            std::cout << std::endl;
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
}