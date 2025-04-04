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
        delete tagged;
        delete ev;
    }
    close(_epfd);
    _events.clear();
}

void Epoll::addServer(int fd, toolbox::SharedPtr<Server> server) {
    struct epoll_event* ev = new struct epoll_event;
    ev->events = EPOLLIN;
    taggedEventData* tagged = new taggedEventData;
    tagged->server = server;
    ev->data.ptr = static_cast<void*>(tagged);
    toolbox::setNonBlocking(fd);
    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, ev) == -1) {
        delete tagged;
        delete ev;
        throw EpollException("epoll_ctl failed");
    }
    _events[fd] = ev;
}

void Epoll::addClient(int fd, toolbox::SharedPtr<Client> client) {
    struct epoll_event* ev = new struct epoll_event;
    ev->events = EPOLLIN | EPOLLET;
    taggedEventData* tagged = new taggedEventData;
    tagged->client = client;
    ev->data.ptr = static_cast<void*>(tagged);
    toolbox::setNonBlocking(fd);
    std::cout << "Epoll::addClient fd: " << client->getFd() << std::endl;
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
        delete tagged;
        delete ev;
        _events.erase(it);
    }
    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        close (fd);
        throw EpollException("epoll_ctl failed");
    }
    close(fd);
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
