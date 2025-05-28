#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include "../../../src/http/response/response.hpp"

int main() {
    // Create a Response object
    http::Response response;

    // Set the status code
    response.setStatus(418);

    // Set some headers
    response.setHeader("Server", "MyServer/1.0");
    response.setHeader("Date", "Wed, 21 Oct 2015 07:28:00 GMT");
    response.setHeader("Location", "http://example.com");
    response.setHeader("Content-Type", "text/html");
    response.setHeader("Connection", "keep-alive");

    // Set the body
    std::string body = "<html><body><h1>Hello, World!</h1></body></html>";
    response.setBody(body);

    // Simulate sending the response to a client
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        close(server_fd);
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 1) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        std::cerr << "Failed to accept client connection" << std::endl;
        close(server_fd);
        return -1;
    }
    std::cout << "Client connected" << std::endl;
    response.sendResponse(client_fd);
    std::cout << "status: " << response.getStatus() << std::endl;
    std::cout << "Content-Length: " << response.getContentLength() << std::endl;
    std::cout << "=== Response Body ===" << std::endl;
    std::cout << response.getBody() << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << "Response sent to client" << std::endl;
    close(client_fd);
    close(server_fd);
    std::cout << "Server closed" << std::endl;

    return 0;
}