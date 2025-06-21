// Copyright 2025 Ideal Broccoli

#pragma once

#include <netinet/in.h>

#include <exception>
#include <string>
#include <ctime>

#include "../../toolbox/shared.hpp"
#include "constant.hpp"

namespace http {
class Request;
}

class Client {
 public:
    class ClientException : public std::exception {
     public:
        explicit ClientException(const char* message);
        ClientException(const ClientException& other);
        virtual ~ClientException() throw();
        const char* what() const throw();
     private:
        ClientException();
        ClientException& operator=(const ClientException& other);
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
    std::size_t getServerPort() const;
    void setLastAccessTime();

    toolbox::SharedPtr<http::Request> getRequest() const;
    void setRequest(const toolbox::SharedPtr<http::Request> request);
    bool isBadRequest() const;
    bool isResponseSending() const;
    bool isCgiProcessing() const;
    bool isClientTimedOut() const;

 private:
    Client();

    int _socket_fd;
    struct sockaddr_in _client_addr;
    socklen_t _client_addr_len;
    time_t _lastAccessTime;
    toolbox::SharedPtr<http::Request> _request;

    std::string convertIpToString(uint32_t ip) const;
};
