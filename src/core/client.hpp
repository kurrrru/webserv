// Copyright 2025 Ideal Broccoli

#pragma once

#include <netinet/in.h>

#include <exception>
#include <string>

#include "../../toolbox/shared.hpp"

namespace http {
class Request;
}

class Client {
 public:
    class ClientException : public std::exception {
     public:
        explicit ClientException(const char* message);
        const char* what() const throw();
     private:
        const char* _message;
    };
    Client(int fd, const struct sockaddr_in& client_addr,
        socklen_t client_addr_len);
    Client(const Client& other);
    Client& operator=(const Client& other);
    virtual ~Client();

    int getFd() const;
    std::string getIp() const;
    std::string getServerIp() const;
    size_t getServerPort() const;

    toolbox::SharedPtr<http::Request> getRequest() const;
    void setRequest(const toolbox::SharedPtr<http::Request> request);
    bool isBadRequest() const;
    bool isResponseSending() const;
    bool isCgiProcessing() const;

 private:
    Client();

    int _socket_fd;
    struct sockaddr_in _client_addr;
    socklen_t _client_addr_len;
    toolbox::SharedPtr<http::Request> _request;

    std::string convertIpToString(uint32_t ip) const;
};
