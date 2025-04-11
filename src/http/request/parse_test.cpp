#include <iostream>

#include "../http_namespace.hpp"
#include "../status_code.hpp"
// #include "parse_request.hpp"
#include "request_parser.hpp"

/*
sample
*/

void showAll(HTTPRequest& r) {
    std::cout << "----- Request line ----" << std::endl;
    std::cout << "method: " << r.method << std::endl;
    std::cout << "path: " << r.uri.path << std::endl;
    std::cout << "version: " << r.version << std::endl;
    std::cout << "----- URI query map ----" << std::endl;
    for (std::map<std::string, std::string>::iterator it = r.uri.queryMap.begin(); it != r.uri.queryMap.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "----- Fields ----" << std::endl;
    for (std::map<std::string, std::vector<std::string>>::iterator it1 =
             r.fields._fieldsMap.begin();
         it1 != r.fields._fieldsMap.end(); ++it1) {
        if (it1->second.empty()) {
            continue;
        }
        std::cout << it1->first << " ->> ";
        for (std::vector<std::string>::iterator it2 = it1->second.begin();
             it2 != it1->second.end(); ++it2) {
            std::cout << *it2;
        }
        std::cout << std::endl;
    }
    std::cout << "----- recv body -----" << std::endl;
    std::cout << r.body.content << std::endl;
}

int main(void) {
    try {
        RequestParser parse;
        std::string sample =
            "GET /index.html?id=5&date=2023-04-09&author=yooshima&limit=10 "
            "HTTP/1.1\r\nHost: example.com\r\nUser-Agent: "
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\nAccept: "
            "text/html,application/xhtml+xml\r\nAccept-Language: "
            "en-US,en;q=0.9\r\nConnection: keep-alive\r\ncontent-length: "
            "10\r\n\r\nbodyy+++++bodyy+++++bodyy";
        std::cout << "----- sample request ----\n" << sample << std::endl;
        parse.run(sample);
        showAll(parse.get());
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}