// Copyright 2025 Ideal Broccoli
#pragma once

#include <fcntl.h>
#include <exception>
#include <string>

class Server {
 public:
    class ServerException : public std::exception {
     public:
        explicit ServerException(const char* message);
        const char* what() const throw();
     private:
        const char* _message;
    };
    Server();
    explicit Server(int port);
    Server(const Server& other);
    Server& operator=(const Server& other);
    virtual ~Server();
    int getFd() const;
    void setName(const std::string& name);
    std::string getName() const;

 private:
    static const int default_port = 8080;
    int _port;
    int _server_sock;
    std::string _name;
    void createServerSocket();
};
