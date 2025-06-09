#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cerrno>
#include <cstdio>
#include <sstream>
#include <cstring>

#include "server.hpp"
#include "client.hpp"
#include "constant.hpp"
#include "../config/config_namespace.hpp"
#include "../config/config_parser.hpp"
#include "../event/epoll.hpp"
#include "../event/tagged_epoll_event.hpp"
#include "../../toolbox/string.hpp"
#include "../../toolbox/shared.hpp"
#include "../http/request/request.hpp"
#include "../http/request/io_pending_state.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc == 1) {
            config::Config::loadConfig(config::DEFAULT_FILE);
        } else if (argc == 2) {
            config::Config::loadConfig(argv[1]);
        }
        toolbox::SharedPtr<config::HttpConfig> httpConfig =
                                        config::Config::getHttpConfig();
        std::vector<toolbox::SharedPtr<Server> > servers;
        for (size_t i = 0; i < httpConfig->getServers().size(); ++i) {
            toolbox::SharedPtr<config::ServerConfig> serverConfig =
                                                    httpConfig->getServers()[i];
            for (size_t j = 0; j < serverConfig->getListens().size(); ++j) {
                int port = serverConfig->getListens()[j].getPort();
                std::string ip = serverConfig->getListens()[j].getIp();
                    toolbox::SharedPtr<Server> server(new Server(port, ip));
                    server->setName(
                        serverConfig->getServerNames()[0].getName());
                    Epoll::addServer(server->getFd(), server);
                    servers.push_back(server);
            }
        }
        int cnt = 0;  // for debug
        struct epoll_event events[1000];
        while (1) {
            // usleep 1ms saves CPU usage 
            usleep(1000);
            try {
                int nfds = Epoll::wait(events, 1000, -1);
                if (nfds == -1) {
                    throw std::runtime_error("epoll_wait failed");
                }
                for (int i = 0; i < nfds; i++) {
                    taggedEventData* tagged =
                        static_cast<taggedEventData*>(events[i].data.ptr);
                    if (tagged->server) {
                        try {
                            std::cout <<"---------------   server   ---------------" <<std::endl;
                            toolbox::SharedPtr<Server> server = tagged->server;
                            struct sockaddr_in client_addr;
                            socklen_t addr_len = sizeof(client_addr);
                            int client_sock = accept(server->getFd(), (struct sockaddr*)&client_addr, &addr_len);
                            if (client_sock == -1) {
                                //continue?
                                throw std::runtime_error("accept failed");
                            }
                            std::cout << server->getName() << " accepted client fd: " << client_sock << std::endl;
                            toolbox::SharedPtr<Client> client(new Client(client_sock, client_addr, addr_len));
                            client->setRequest(toolbox::SharedPtr<http::Request>(new http::Request(client)));
                            Epoll::addClient(client_sock, client); // this func will throw exception
                        } catch(std::exception& e) {
                            std::cerr << e.what() << std:: endl;
                        }
                    } else {
                        try {
                            std::cout << "--------------- client " << ++cnt << " ---------------" <<std::endl;
                            toolbox::SharedPtr<Client> client = tagged->client;
                            int client_sock = client->getFd();
                            std::cout << "send response to client fd: " << client_sock << std::endl;

                            if (isSocketDisconnected(events[i])) {
                                Epoll::del(client_sock);
                                continue;
                            } else if (events[i].events & EPOLLOUT ||
                                (events[i].events & EPOLLIN && client->getRequest()->getIOPendingState() != http::RESPONSE_SENDING)) {
                                client->getRequest()->run();
                            }
                            // If the client is not keep-alive and the response is complete or bad request
                            if (client->isOnceConnectionEnd() || client->isBadRequest()) {
                                Epoll::del(client_sock);
                            } else if (client->getRequest()->getIOPendingState() == http::END_RESPONSE) {
                                client->clearRequest(client);
                            }
                        } catch (std::exception& e) {
                            std::cerr << e.what() << std:: endl;
                        }
                    }
                }
            } catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        for (size_t i = 0; i < servers.size(); ++i) {
            Epoll::del(servers[i]->getFd());
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
