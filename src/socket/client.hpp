// Copyright 2025 Ideal Broccoli

#pragma once

#include <exception>

class Client {
 public:
    class ClientException : public std::exception {
     public:
        explicit ClientException(const char* message);
        const char* what() const throw();
     private:
        const char* _message;
    };
    Client();
    explicit Client(int fd);
    Client(const Client& other);
    Client& operator=(const Client& other);
    virtual ~Client();
    int getFd() const;
 private:
    int _socket_fd;
};
