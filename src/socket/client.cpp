// Copyright 2025 Ideal Broccoli

#include "client.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

Client::Client() : _socket_fd(-1) {
}

Client::Client(int fd): _socket_fd(fd) {
}

Client::Client(const Client& other): _socket_fd(other._socket_fd) {
}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
        _socket_fd = other._socket_fd;
    }
    return *this;
}

Client::~Client() {
    if (_socket_fd != -1) {
        close(_socket_fd);
    }
}

Client::ClientException::ClientException(const char* message) :
                            _message(message) {}

const char* Client::ClientException::what() const throw() {
    return _message;
}

int Client::getFd() const {
    return _socket_fd;
}
