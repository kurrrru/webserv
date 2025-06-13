#include "../../../src/http/get_gmt.hpp"

#include <iostream>

int main() {
    std::string httpDate = http::getCurrentGMT();
    if (httpDate.empty()) {
        std::cerr << "Failed to get HTTP date." << std::endl;
        return 1;
    }
    std::cout << "Current HTTP date: " << httpDate << std::endl;
    return 0;
}
