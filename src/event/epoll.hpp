// Copyright 2025 Ideal Broccoli

#pragma once

#include <sys/epoll.h>
#include <exception>
#include <map>

#include "../core/server.hpp"
#include "../core/client.hpp"
#include "../../toolbox/shared.hpp"

class Epoll {
 public:
    class EpollException : public std::exception {
     public:
        explicit EpollException(const char* message);
        const char* what() const throw();
     private:
        const char* _message;
    };
    Epoll();
    ~Epoll();
    void addServer(int fd, toolbox::SharedPtr<Server> server);
    void addClient(int fd, toolbox::SharedPtr<Client> client);
    void del(int fd);
    int wait(struct epoll_event* events, int maxevents, int timeout);
 private:
    int _epfd;
    std::map<int, struct epoll_event*> _events;
};
