#include <iostream>

#include "../http_namespace.hpp"
#include "../status_code.hpp"
#include "parse_request.hpp"

/*
sample
*/

int main(void) {
  try {
    ParseRequest parse;
    std::string sample =
        "GET /index.html?id=5 HTTP/1.1\r\nHost: example.com\r\nUser-Agent: "
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\nAccept: "
        "text/html,application/xhtml+xml\r\nAccept-Language: "
        "en-US,en;q=0.9\r\nConnection: keep-alive\r\ncontent-length: "
        "10\r\n\r\nbodyy+++++bodyy+++++bodyy";
    std::cout << "----- sample request ----\n" << sample << std::endl;
    parse.run(sample);
    parse.showAll();
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}