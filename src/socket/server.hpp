#pragma once

#include <string>
#include <fcntl.h>

class Server {
 public:
    Server();
    Server(int port);
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
