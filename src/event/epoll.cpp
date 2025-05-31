// Copyright 2025 Ideal Broccoli

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <iostream>
#include <map>

#include "epoll.hpp"
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
    for (std::map<int, struct epoll_event*>::iterator it = _events.begin();
            it != _events.end(); ++it) {
        struct epoll_event* ev = it->second;
        taggedEventData* tagged = static_cast<taggedEventData*>(ev->data.ptr);
        delete tagged;
        delete ev;
    }
    close(_epfd);
    _events.clear();
}

void Epoll::addServer(int fd, toolbox::SharedPtr<Server> server) {
    Epoll& epollInstance = getInstance();
    struct epoll_event* ev = new struct epoll_event;
    ev->events = EPOLLIN;
    taggedEventData* tagged = new taggedEventData;
    tagged->server = server;
    ev->data.ptr = static_cast<void*>(tagged);
    toolbox::setNonBlocking(fd);
    if (epoll_ctl(epollInstance._epfd, EPOLL_CTL_ADD, fd, ev) == -1) {
        delete tagged;
        delete ev;
        throw EpollException("epoll_ctl failed");
    }
    epollInstance._events[fd] = ev;
}

void Epoll::addClient(int fd, toolbox::SharedPtr<Client> client) {
    Epoll& epollInstance = getInstance();
    struct epoll_event* ev = new struct epoll_event;
    ev->events = EPOLLIN;
    taggedEventData* tagged = new taggedEventData;
    tagged->client = client;
    ev->data.ptr = static_cast<void*>(tagged);
    toolbox::setNonBlocking(fd);
    if (epoll_ctl(epollInstance._epfd, EPOLL_CTL_ADD, fd, ev) == -1) {
        delete tagged;
        delete ev;
        throw EpollException("epoll_ctl failed");
    }
    epollInstance._events[fd] = ev;
}

void Epoll::del(int fd) {
    Epoll& epollInstance = getInstance();
    std::map<int, struct epoll_event*>::iterator it = epollInstance._events.find(fd);
    if (it != epollInstance._events.end()) {
        struct epoll_event* ev = it->second;
        taggedEventData* tagged = static_cast<taggedEventData*>(ev->data.ptr);
        delete tagged;
        delete ev;
        epollInstance._events.erase(it);
    }
    if (epoll_ctl(epollInstance._epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        close(fd);
        throw EpollException("epoll_ctl failed");
    }
    close(fd);
}

int Epoll::wait(struct epoll_event* events, int maxevents, int timeout) {
    Epoll& epollInstance = getInstance();
    return epoll_wait(epollInstance._epfd, events, maxevents, timeout);
}

Epoll& Epoll::getInstance() {
    static Epoll instance;
    return instance;
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
