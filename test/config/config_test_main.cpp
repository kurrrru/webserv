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
            std::cout << "listen ip: " << http->servers[i]->listen.ip << std::endl;
            std::cout << "listen port: " << http->servers[i]->listen.port << std::endl;
            std::cout << "server_name: ";
            for (size_t j = 0; j < http->servers[i]->server_names.size(); ++j) {
                const std::vector<std::string>& names = http->servers[i]->server_names[j].names;
                for (size_t k = 0; k < names.size(); ++k) {
                    std::cout << names[k];
                    if (k < names.size() - 1) {
                        std::cout << " ";
                    }
                }
                if (j < http->servers[i]->server_names.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
            printSettings("", *http->servers[i]);
            for (size_t j = 0; j < http->servers[i]->locations.size(); ++j) {
                std::cout << "===== LOCATION =====" << std::endl;
                std::cout << "location #" << (j + 1) << ":" << std::endl;
                std::cout << "path: " << http->servers[i]->locations[j]->path << std::endl;
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