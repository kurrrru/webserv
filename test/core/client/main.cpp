// this is a simple version of src/core/main.cpp

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

#include "../../../src/core/server.hpp"
#include "../../../src/core/client.hpp"
#include "../../../src/event/epoll.hpp"
#include "../../../src/event/tagged_epoll_event.hpp"
#include "../../../toolbox/string.hpp"
#include "../../../toolbox/shared.hpp"

int main(void) {
    try {
        Epoll epoll;
        toolbox::SharedPtr<Server> server1(new Server(3000));
        server1->setName("server1");
        epoll.addServer(server1->getFd(), server1);
        toolbox::SharedPtr<Server> server2(new Server(5000));
        server2->setName("server2");
        epoll.addServer(server2->getFd(), server2);

        int cnt = 0;
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
                                throw std::runtime_error("accept failed");
                            }
                            std::cout << server->getName() << " accepted client fd: " << client_sock << std::endl;
                            toolbox::SharedPtr<Client> client(new Client(client_sock, client_addr, addr_len));
                            epoll.addClient(client_sock, client);
                        } catch(std::exception& e) {
                            std::cerr << e.what() << std:: endl;
                        }
                    } else {
                        try {
                            std::cout << "--------------- client " << ++cnt << " ---------------" <<std::endl;
                            toolbox::SharedPtr<Client> client = tagged->client;
                            int client_sock = client->getFd();
                            std::cout << "send response to client fd: " << client_sock << std::endl;
                            std::cout << "client ip: " << client->getIp() << std::endl;
                            std::cout << "server ip: " << client->getServerIp() << std::endl;
                            std::cout << "server port: " << client->getServerPort() << std::endl;

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

                            const char* responseHeader = "HTTP/1.1 200 OK\r\nContent-Length: ";
                            std::string responseBody = "<html><body>hello\n" + whole_request + "</body></html>";
                            std::string response = responseHeader + toolbox::to_string(responseBody.size()) + "\r\n\r\n" + responseBody;
                            if (send(client_sock, response.c_str(), response.size(), 0) == -1) {
                                throw std::runtime_error("send failed");
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
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
