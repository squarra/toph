#include "frame.h"
#include <iostream>

int main() {
    toph::Frame link_1("link_1");
    toph::Frame link_2("link_2", &link_1);

    std::cout << link_2 << std::endl;
    return 0;
}
