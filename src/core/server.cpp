// Copyright 2025 Ideal Broccoli

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "server.hpp"
#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace server {
const int DEFAULT_PORT = 8080;
const char* DEFAULT_NAME = "server";
const char* DEFAULT_IP = "0.0.0.0";
}

Server::Server() {
    _port = server::DEFAULT_PORT;
    _ip = server::DEFAULT_IP;
    _name = server::DEFAULT_NAME;
    createServerSocket();
}

Server::Server(int port) {
    _port = port;
    _ip = server::DEFAULT_IP;
    _name = server::DEFAULT_NAME;
    createServerSocket();
}

Server::Server(int port, const std::string& ip) {
    _port = port;
    _ip = ip;
    _name = server::DEFAULT_NAME;
    createServerSocket();
}

Server::~Server() {
    close(_server_sock);
}

Server::ServerException::ServerException(const char* message) :
_message(message) {}

Server::ServerException::ServerException(const std::string& message) :
_message(message) {}

Server::ServerException::ServerException(const ServerException& other) :
_message(other._message) {}

Server::ServerException::~ServerException() throw() {
}

const char* Server::ServerException::what() const throw() {
    return _message.c_str();
}

void Server::createServerSocket() {
    _server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_sock == -1) {
        throw ServerException("socket failed");
    }
    int opt = 1;
    if (setsockopt(_server_sock, SOL_SOCKET, SO_REUSEADDR,
                    &opt, sizeof(opt)) == -1) {
        throw ServerException("setsockopt failed");
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = parseIpAddress(_ip);
    server_addr.sin_port = htons(_port);
    if (bind(_server_sock, (struct sockaddr*)&server_addr,
                sizeof(server_addr)) == -1) {
        std::string errorMsg = "bind() to " + _ip + ":"
                                + toolbox::to_string(_port) + " failed";
        toolbox::logger::StepMark::error(errorMsg);
        throw ServerException(errorMsg);
    }
    if (listen(_server_sock, SOMAXCONN) == -1) {
        throw ServerException("listen failed");
    }
}

uint32_t Server::parseIpAddress(const std::string& ip) const {
    if (ip == "0.0.0.0") {
        return INADDR_ANY;
    }
    std::size_t pos1 = ip.find('.');
    std::size_t pos2 = ip.find('.', pos1 + 1);
    std::size_t pos3 = ip.find('.', pos2 + 1);
    if (pos1 == std::string::npos ||
        pos2 == std::string::npos ||
        pos3 == std::string::npos) {
        std::string errorMsg = "invalid IP address format: " + ip;
        toolbox::logger::StepMark::error(errorMsg);
        throw ServerException(errorMsg);
    }
    int a = std::atoi(ip.substr(0, pos1).c_str());
    int b = std::atoi(ip.substr(pos1 + 1, pos2 - pos1 - 1).c_str());
    int c = std::atoi(ip.substr(pos2 + 1, pos3 - pos2 - 1).c_str());
    int d = std::atoi(ip.substr(pos3 + 1).c_str());
    int octets[4] = {a, b, c, d};
    for (int i = 0; i < 4; ++i) {
        if (octets[i] < 0 || octets[i] > 255) {
            std::string errorMsg = "invalid IP address range: " + ip;
            toolbox::logger::StepMark::error(errorMsg);
            throw ServerException(errorMsg);
        }
    }
    return htonl((a << 24) | (b << 16) | (c << 8) | d);
}
