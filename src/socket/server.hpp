#pragma once

#include <string>
#include <fcntl.h>

class Server {
 public:
    Server();
    Server(int port);
    Server(Server const& other);
    Server& operator=(Server const& other);
    virtual ~Server();
    int getFd();
    void setName(const std::string& name);
    std::string getName();
 private:
    static const int default_port = 8080;
    int _port;
    int _server_sock;
    std::string _name;
    void createServerSocket();
};
