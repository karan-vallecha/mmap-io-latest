#include <iostream>
#include "src/mmap-utils.h"

int main() {
    try {
        MMapFile file("test.txt", false); // false = read-only
        std::cout << "File size: " << file.getSize() << " bytes\n";
        std::cout << "Contents: " << std::string((char*)file.getData(), file.getSize()) << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
