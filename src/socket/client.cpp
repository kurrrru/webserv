#include "client.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

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

int Client::getFd() const {
    return _socket_fd;
}
