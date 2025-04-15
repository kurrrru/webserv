// Copyright 2025 Ideal Broccoli

#include "server.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <iostream>

Server::Server() {
    _port = default_port;
    _name = "server";
    createServerSocket();
}

Server::Server(int port) {
    _port = port;
    _name = "server";
    createServerSocket();
}

Server::Server(const Server& other) {
    _port = other._port;
}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        _port = other._port;
    }
    return *this;
}

Server::~Server() {
    close(_server_sock);
}

Server::ServerException::ServerException(const char* message) :
                            _message(message) {}

const char* Server::ServerException::what() const throw() {
    return _message;
}

int Server::getFd() const {
    return _server_sock;
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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);

    if (bind(_server_sock, (struct sockaddr*)&server_addr,
                sizeof(server_addr)) == -1) {
        throw ServerException("bind failed");
    }

    if (listen(_server_sock, SOMAXCONN) == -1) {
        throw ServerException("listen failed");
    }
}

void Server::setName(const std::string& name) {
    _name = name;
}

std::string Server::getName() const {
    return _name;
}
