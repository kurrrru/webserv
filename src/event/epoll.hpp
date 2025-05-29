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
         explicit EpollException(const char* message) : _message(message) {}
         const char* what() const throw() { return _message; }
     private:
         const char* _message;
     };

    static void addServer(int fd, toolbox::SharedPtr<Server> server);
    static void addClient(int fd, toolbox::SharedPtr<Client> client);
    static void del(int fd);
    static int wait(struct epoll_event* events, int maxevents, int timeout);

 private:
    Epoll();
    ~Epoll();
    Epoll(const Epoll&) {};
    Epoll& operator=(const Epoll&) { return *this; }

    static Epoll& getInstance();

    int _epfd;
    std::map<int, struct epoll_event*> _events;
};
