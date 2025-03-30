#include "epoll.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "tagged_epoll_event.hpp"

namespace toolbox {
    static void setNonBlocking(int fd);
}

Epoll::Epoll() {
    _epfd = epoll_create(1);
    if (_epfd == -1) {
        throw EpollException("epoll_create failed");
    }
}

Epoll::~Epoll() {
    for (std::map<int, struct epoll_event*>::iterator it = _events.begin(); it != _events.end(); ++it) {
        struct epoll_event* ev = it->second;
        taggedEventData* tagged = static_cast<taggedEventData*>(ev->data.ptr);
        if (tagged->tag == "server") {
            Server* server = static_cast<Server*>(tagged->ptr);
            delete server;
        } else if (tagged->tag == "client") {
            Client* client = static_cast<Client*>(tagged->ptr);
            delete client;
        }
        delete tagged;
        delete ev;
    }
    close(_epfd);
    _events.clear();
}

void Epoll::addServer(int fd, Server* server) {
    struct epoll_event* ev = new struct epoll_event;
    ev->events = EPOLLIN;

    taggedEventData* tagged = new taggedEventData;
    tagged->ptr = server;
    tagged->tag = "server";
    ev->data.ptr = tagged;
    toolbox::setNonBlocking(fd);
    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, ev) == -1) {
        delete tagged;
        delete ev;
        throw EpollException("epoll_ctl failed");
    }
    _events[fd] = ev;
}

void Epoll::addClient(int fd, Client* client) {
    struct epoll_event* ev = new struct epoll_event;
    ev->events = EPOLLIN | EPOLLET;
    taggedEventData* tagged = new taggedEventData;
    tagged->ptr = client;
    tagged->tag = "client";
    ev->data.ptr = tagged;
    toolbox::setNonBlocking(fd);
    std::cout << "Epoll::addClient fd = " << static_cast<Client*>(tagged->ptr)->getFd() << std::endl;
    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, ev) == -1) {
        delete tagged;
        delete ev;
        throw EpollException("epoll_ctl failed");
    }
    _events[fd] = ev;
}

void Epoll::del(int fd) {
    std::map<int, struct epoll_event*>::iterator it = _events.find(fd);
    if (it != _events.end()) {
        struct epoll_event* ev = it->second;
        taggedEventData* tagged = static_cast<taggedEventData*>(ev->data.ptr);
        if (tagged->tag == "server") {
            Server* server = static_cast<Server*>(tagged->ptr);
            delete server;
        } else if (tagged->tag == "client") {
            Client* client = static_cast<Client*>(tagged->ptr);
            delete client;
        }
        delete tagged;
        delete ev;
        _events.erase(it);
    }
    epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
}

int Epoll::wait(struct epoll_event* events, int maxevents, int timeout) {
    return epoll_wait(_epfd, events, maxevents, timeout);
}

Epoll::EpollException::EpollException(const char* message) : _message(message) {}

const char* Epoll::EpollException::what() const throw() {
    return _message;
}

static void toolbox::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("fcntl F_GETFL failed");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("fcntl F_SETFL failed");
    }
}