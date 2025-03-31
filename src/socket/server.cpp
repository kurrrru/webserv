#include "server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>

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

int Server::getFd() const {
    return _server_sock;
}

void Server::createServerSocket() {
    _server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(_server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);

    if (bind(_server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(_server_sock, SOMAXCONN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

void Server::setName(const std::string& name) {
    _name = name;
}

std::string Server::getName() const {
    return _name;
}