#include "frame.h"
#include <iostream>
#include <memory>

int main() {
    std::shared_ptr<toph::Frame> frame = std::make_shared<toph::Frame>("hello");
    std::cout << *frame.get() << std::endl;
}