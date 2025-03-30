#pragma once

#include <sys/epoll.h>
#include <exception>
#include <map>

#include "../socket/server.hpp"
#include "../socket/client.hpp"

class Epoll {
 public:
    class EpollException : public std::exception {
     public:
        EpollException(const char* message);
        const char* what() const throw();
     private:
        const char* _message;
    };
    Epoll();
    ~Epoll();
    void addServer(int fd, Server* server);
    void addClient(int fd, Client* client);
    void del(int fd);
    int wait(struct epoll_event* events, int maxevents, int timeout);
 private:
    int _epfd;
    std::map<int, struct epoll_event*> _events;
};
