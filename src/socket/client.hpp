#pragma once

class Client {
 public:
    Client();
    Client(int fd);
    Client(const Client& other);
    Client& operator=(const Client& other);
    virtual ~Client();
    int getFd() const;
 private:
    int _socket_fd;
};
