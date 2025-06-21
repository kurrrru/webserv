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

std::string g_filterDirective = "";

bool shouldPrintDirective(const std::string& directiveName) {
    if (g_filterDirective.empty()) {
        return true;
    }
    return g_filterDirective == directiveName;
}

void printSettings(const std::string& prefix, const config::ConfigBase& conf) {
    if (shouldPrintDirective("allowed_methods")) {
        std::cout << prefix << "allowed_methods: ";
        const std::vector<std::string>& allowedMethods = conf.getAllowedMethods();
        for (std::size_t i = 0; i < allowedMethods.size(); ++i) {
            std::cout << allowedMethods[i];
            if (i < allowedMethods.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    if (shouldPrintDirective("autoindex")) {
        std::cout << prefix << "autoindex: " << (conf.getAutoindex() ? "on" : "off") << std::endl;
    }
    if (shouldPrintDirective("cgi_extension")) {
        std::cout << prefix << "cgi_extension: ";
        const std::vector<std::string>& cgiExtensions = conf.getCgiExtensions();
        for (std::size_t i = 0; i < cgiExtensions.size(); ++i) {
            std::cout << cgiExtensions[i];
            if (i < cgiExtensions.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    if (shouldPrintDirective("cgi_path")) {
        std::cout << prefix << "cgi_path: " << conf.getCgiPath() << std::endl;
    }
    if (shouldPrintDirective("client_max_body_size")) {
        std::cout << prefix << "client_max_body_size: " << conf.getClientMaxBodySize() << std::endl;
    }
    if (shouldPrintDirective("error_page")) {
        std::cout << prefix << "error_page: ";
        const std::vector<config::ErrorPage>& errorPages = conf.getErrorPages();
        for (std::size_t i = 0; i < errorPages.size(); ++i) {
            std::cout << "[";
            for (std::size_t j = 0; j < errorPages[i].getCodes().size(); ++j) {
                std::cout << errorPages[i].getCodes()[j];
                if (j < errorPages[i].getCodes().size() - 1) std::cout << ", ";
            }
            std::cout << "] -> " << errorPages[i].getPath();
            if (i < errorPages.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    if (shouldPrintDirective("index")) {
        std::cout << prefix << "index: ";
        const std::vector<std::string>& indices = conf.getIndices();
        for (std::size_t i = 0; i < indices.size(); ++i) {
            std::cout << indices[i];
            if (i < indices.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    if (shouldPrintDirective("root")) {
        std::cout << prefix << "root: " << conf.getRoot() << std::endl;
    }
    if (shouldPrintDirective("upload_store")) {
        std::cout << prefix << "upload_store: " << conf.getUploadStore() << std::endl;
    }
    if (g_filterDirective.empty()) {
        std::cout << std::endl;
    }
}

void printLocationRecursively(const toolbox::SharedPtr<config::LocationConfig>& location, int depth = 0) {
    std::cout << "===== LOCATION (depth: " << depth << ") =====" << std::endl;
    std::cout << "location path: " << location->getPath() << std::endl;
    std::cout << "# Child locations: " << location->getLocations().size() << std::endl;
    for (std::size_t i = 0; i < location->getLocations().size(); ++i) {
        std::cout << "# Child " << i << " path: " << location->getLocations()[i]->getPath() << std::endl;
    }
    if (shouldPrintDirective("return") && location->getReturnValue().hasReturnValue()) {
        std::cout << "return code: " << location->getReturnValue().getStatusCode() << std::endl;
        if (location->getReturnValue().isTextOrUrlSetting()) {
            std::cout << "return text_or_url: " << location->getReturnValue().getTextOrUrl() << std::endl;
        }
    }
    printSettings("", *location);
    for (std::size_t i = 0; i < location->getLocations().size(); ++i) {
        printLocationRecursively(location->getLocations()[i], depth + 1);
    }
}


int main(int argc, char* argv[]) {
    toolbox::logger::StepMark::setLogFile("config_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    if (argc >= 3) {
        g_filterDirective = argv[2];
    }
    try {
        try {
            if (argc == 1) {
                config::Config::loadConfig("../../conf/default.conf");
            } else if (argc >= 2) {
                config::Config::loadConfig(argv[1]);
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        const toolbox::SharedPtr<config::HttpConfig>& http = config::Config::getHttpConfig();
        if (!http) {
            return EXIT_SUCCESS;
        }
        std::cout << "===== HTTP =====" << std::endl;
        printSettings("", *http);
        for (std::size_t i = 0; i < http->getServers().size(); ++i) {
            std::cout << "===== SERVER =====" << std::endl;
            std::cout << "server #" << (i + 1) << std::endl;
            if (shouldPrintDirective("listen")) {
                std::cout << "listen: ";
                for (std::size_t listenIdx = 0; listenIdx < http->getServers()[i]->getListens().size(); ++listenIdx) {
                    const config::Listen& listenConfig = http->getServers()[i]->getListens()[listenIdx];
                    std::cout << listenConfig.getIp() << ":" << listenConfig.getPort();
                    if (listenConfig.isDefaultServer()) {
                        std::cout << " (default)";
                    }
                    if (listenIdx < http->getServers()[i]->getListens().size() - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << std::endl;
            }
            if (shouldPrintDirective("server_name")) {
                std::cout << "server_name: ";
                for (std::size_t j = 0; j < http->getServers()[i]->getServerNames().size(); ++j) {
                    const config::ServerName& serverName = http->getServers()[i]->getServerNames()[j];
                    if (serverName.getType() == config::ServerName::WILDCARD_START) {
                        std::cout << "*" << serverName.getName();
                    } else if (serverName.getType() == config::ServerName::WILDCARD_END) {
                        std::cout << serverName.getName() << "*";
                    } else {
                        std::cout << serverName.getName();
                    }
                    if (j < http->getServers()[i]->getServerNames().size() - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << std::endl;
            }
            if (shouldPrintDirective("return") && http->getServers()[i]->getReturnValue().hasReturnValue()) {
                std::cout << "return code: " << http->getServers()[i]->getReturnValue().getStatusCode() << std::endl;
                if (http->getServers()[i]->getReturnValue().isTextOrUrlSetting()) {
                    std::cout << "return textOrUrl: " << http->getServers()[i]->getReturnValue().getTextOrUrl() << std::endl;
                }
            }
            printSettings("", *http->getServers()[i]);
            for (std::size_t j = 0; j < http->getServers()[i]->getLocations().size(); ++j) {
                printLocationRecursively(http->getServers()[i]->getLocations()[j]);
            }
            std::cout << std::endl;
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}