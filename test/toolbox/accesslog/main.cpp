
#include "../../../toolbox/access.hpp"

int main() {
    toolbox::logger::AccessLog::setLogFile("special_access.log");
    toolbox::logger::AccessLog::log(
        "127.0.0.1",
        "user",
        "GET /index.html HTTP/1.1",
        200,
        1234,
        "http://example.com",
        "curl/7.64.1"
    );
    toolbox::logger::AccessLog::log(
        "127.0.0.1",
        "user",
        "GET /index.html HTTP/1.1",
        200,
        1234,
        "http://example.com",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3"
    );
    toolbox::logger::AccessLog::setLogFile("access.log");
    toolbox::logger::AccessLog::log(
        "127.0.0.1",
        "-",
        "GET /index.html HTTP/1.1",
        200,
        1234,
        "http://example.com",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3"
    );
}