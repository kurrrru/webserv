// Copyright 2025 Ideal Broccoli

#include "client.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include "../../toolbox/string.hpp"

Client::Client() : _socket_fd(-1) {
}

Client::Client(int fd, const struct sockaddr_in& client_addr,
            socklen_t client_addr_len) :
            _socket_fd(fd), _client_addr(client_addr),
            _client_addr_len(client_addr_len) {
}

Client::Client(const Client& other): _socket_fd(other._socket_fd),
    _client_addr(other._client_addr),
    _client_addr_len(other._client_addr_len) {
}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
        _socket_fd = other._socket_fd;
        _client_addr = other._client_addr;
        _client_addr_len = other._client_addr_len;
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

std::string Client::getIp() const {
    uint32_t ip = ntohl(_client_addr.sin_addr.s_addr);
    std::string ip_str = toolbox::to_string((ip >> 24) & 0xFF) + "." +
        toolbox::to_string((ip >> 16) & 0xFF) + "." +
        toolbox::to_string((ip >> 8) & 0xFF) + "." +
        toolbox::to_string(ip & 0xFF);
    return ip_str;
}
