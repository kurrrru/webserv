// Copyright 2025 Ideal Broccoli
#pragma once

#include <fcntl.h>
#include <exception>
#include <string>
#include <stdint.h> 

namespace server {
extern const int DEFAULT_PORT;
extern const char* DEFAULT_IP;
}  // namespace server

class Server {
 public:
    class ServerException : public std::exception {
     public:
        explicit ServerException(const char* message);
        explicit ServerException(const std::string& message);
        virtual ~ServerException() throw();
        const char* what() const throw();
     private:
        std::string _message;
    };
    Server();
    explicit Server(int port);
    Server(int port, const std::string& ip);
    Server(const Server& other);
    Server& operator=(const Server& other);
    virtual ~Server();

    int getFd() const { return _server_sock; }
    void setName(const std::string& name) { _name = name; }
    std::string getName() const { return _name; }

 private:
    int _port;
    int _server_sock;
    std::string _name;
    std::string _ip;
    void createServerSocket();
    uint32_t parseIpAddress(const std::string& ip) const;
};
