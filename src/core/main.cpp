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
        } else {
            std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
            return EXIT_FAILURE;
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
        struct epoll_event events[1000];
        while (1) {
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
                            toolbox::SharedPtr<Server> server = tagged->server;
                            struct sockaddr_in client_addr;
                            socklen_t addr_len = sizeof(client_addr);
                            int client_sock = accept(server->getFd(), (struct sockaddr*)&client_addr, &addr_len);
                            if (client_sock == -1) {
                                //continue?
                                throw std::runtime_error("accept failed");
                            }
                            toolbox::SharedPtr<Client> client(new Client(client_sock, client_addr, addr_len));
                            client->setRequest(toolbox::SharedPtr<http::Request>(new http::Request(client.get())));
                            Epoll::addClient(client_sock, client); // this func will throw exception
                        } catch(std::exception& e) {
                            toolbox::logger::StepMark::error("Main: server: " + std::string(e.what()));
                        }
                    } else {
                        try {
                            toolbox::SharedPtr<Client> client = tagged->client;
                            int client_sock = client->getFd();

                            if (isSocketDisconnected(events[i])) {
                                Epoll::del(client_sock);
                                continue;
                            } else if ((events[i].events & EPOLLOUT && (client->isResponseSending() || client->isCgiProcessing()))
                                || (events[i].events & EPOLLIN && !client->isResponseSending())) {
                                client->getRequest()->run();
                            }

                            if (client->getRequest()->getIOPendingState() == http::END_RESPONSE) {
                                Epoll::del(client_sock);
                            }
                        } catch (std::exception& e) {
                            toolbox::logger::StepMark::error("Main: client: " + std::string(e.what()));
                        }
                    }
                }
            } catch (std::exception& e) {
                toolbox::logger::StepMark::error("Main: whileloop: " + std::string(e.what()));
            }
        }
        for (size_t i = 0; i < servers.size(); ++i) {
            Epoll::del(servers[i]->getFd());
        }
    } catch (std::exception& e) {
        toolbox::logger::StepMark::critical("Main: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
