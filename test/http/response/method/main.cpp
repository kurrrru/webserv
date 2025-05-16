#include <iostream>

void get_test();
void head_test();
void delete_test();

int main() {
    std::cout << "----------------GET----------------" << std::endl;
    get_test();
    std::cout << "----------------HEAD----------------" << std::endl;
    head_test();
    std::cout << "----------------DELETE----------------" << std::endl;
    delete_test();
    return 0;
}
