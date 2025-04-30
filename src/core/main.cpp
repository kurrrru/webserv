// Copyright 2025 Ideal Broccoli

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

#include "../config/config_parser.hpp"
#include "../socket/server.hpp"
#include "../socket/client.hpp"
#include "../event/epoll.hpp"
#include "../event/tagged_epoll_event.hpp"
#include "../../toolbox/string.hpp"
#include "../../toolbox/shared.hpp"

int main(int argc, char* argv[]) {
    try {
        toolbox::SharedPtr<config::Config> config;
        if (argc == 1) {
            config = config::ConfigParser::parseFile("", true);
        } else if (argc == 2) {
            config = config::ConfigParser::parseFile(argv[1], false);
        }
        toolbox::SharedPtr<config::HttpConfig> http_config = config->getHttpConfig();
        toolbox::SharedPtr<config::ServerConfig> server_config1 = http_config->getServers()[0];
        toolbox::SharedPtr<config::ServerConfig> server_config2 = http_config->getServers()[1];

        Epoll epoll;
        toolbox::SharedPtr<Server> server1(new Server(server_config1->listen.port));
        server1->setName(server_config1->server_names[0].names[0]);
        epoll.addServer(server1->getFd(), server1);

        toolbox::SharedPtr<Server> server2(new Server(server_config2->listen.port));
        server2->setName(server_config2->server_names[0].names[0]);
        epoll.addServer(server2->getFd(), server2);

        int cnt = 0;  // for debug
        struct epoll_event events[1000];
        while (1) {
            try {
                int nfds = epoll.wait(events, 1000, -1);
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
                            toolbox::SharedPtr<Client> client(new Client(client_sock));
                            epoll.addClient(client_sock, client); // this func will throw exception
                        } catch(std::exception& e) {
                            std::cerr << e.what() << std:: endl;
                        }
                    } else {
                        try {
                            std::cout << "--------------- client " << ++cnt << " ---------------" <<std::endl;
                            toolbox::SharedPtr<Client> client = tagged->client;
                            int client_sock = client->getFd();
                            std::cout << "send response to client fd: " << client_sock << std::endl;

                            char buf[1024];
                            int len = 0;
                            std::string whole_request;
                            do {
                                len = recv(client_sock, buf, sizeof(buf), 0);
                                if (len == -1) {
                                    if (errno == EAGAIN) { //if no recv data
                                        break;
                                    }
                                    throw std::runtime_error("recv failed");
                                } else if (len == 0) {
                                    break;
                                } else {
                                    whole_request.append(buf, len);
                                }
                            } while (len > 0);

                            // requestをparseして、適切なresponseを作成する
                            // Response response;
                            // Request* request;
                            // if (method == "GET") {
                            //     request = new GetRequest();
                            // } else if (method == "HEAD") {
                            //     request = new HeadRequest();
                            // } else if (method == "POST") {
                            //     request = new PostRequest();
                            // } else if (method == "DELETE") {
                            //     request = new DeleteRequest();
                            // } else {
                            //     the method is not supported
                            // }
                            // parseした情報を反映
                            // request->run(&response);
                            // std::string response = response->getResponse();
                            // send(client_sock, response.c_str(), response.size(), 0);

                            const char* responseHeader = "HTTP/1.1 200 OK\r\nContent-Length: ";
                            std::string responseBody = "<html><body>hello" + whole_request + "</body></html>";
                            std::string response = responseHeader + toolbox::to_string(responseBody.size()) + "\r\n\r\n" + responseBody;
                            if (send(client_sock, response.c_str(), response.size(), 0) == -1) {
                                throw std::runtime_error("send failed");
                                // Send exit status to client
                            }
                            epoll.del(client_sock);
                        } catch (std::exception& e) {
                            std::cerr << e.what() << std:: endl;
                        }
                    }
                }
            } catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        epoll.del(server1->getFd());
        epoll.del(server2->getFd());
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
