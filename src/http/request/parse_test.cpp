#include <iostream>
#include "parse_request.hpp"
#include "http_namespace.hpp"

/*
sample
*/

int main(void) {
    try {

        RequestParse parse;
        std::string sample = "GET /index.html HTTP/1.1\r\n\
                            Host: example.com\r\n\
                            User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n\
                            Accept: text/html,application/xhtml+xml\r\n\
                            Accept-Language: en-US,en;q=0.9\r\n\
                            Connection: keep-alive\r\n\r\nbody";
        std::cout << "Sample input" << std::endl;
        parse.run(sample);
        // RequestData data;
        // data.inputBuffer = sample;
        // Request::run(data);
        // std::cout << "METHOD: " << data.method << std::endl;
        // std::cout << "URI: " << data.uri << std::endl;
        // std::cout << "HTTP/VERSION: " << data.httpVersion << std::endl;
        // for (std::map<std::string, std::vector<std::string> >::iterator it = data.fields.begin(); it != data.fields.end(); ++it) {
        //     std::cout << "FIELDS: " << it->first << ": ";
        //     std::vector<std::string>& value = it->second;
        //     for (size_t i = 0; i < value.size(); ++i) {
        //         std::cout << value[i];
        //         if (i < value.size() - 1) {
        //             std::cout << ", ";
        //         }
        //     }
        //     std::cout << std::endl;
        // }
        // std::cout << "BODY: " << data.body << std::endl;
    } catch(std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}
