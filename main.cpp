#include "compare_files.hpp"

#include <iostream>


int main() {
    std::string directory1, directory2;
    std::cout << "Enter directory 1: ";
    std::cin >> directory1;
    std::cout << "Enter directory 2: ";
    std::cin >> directory2;
    float threshold;
    std::cout << "Enter threshold for similar files (float number in (0, 1]): ";
    std::cin >> threshold;

    if (threshold <= 0 || threshold > 1) {
        std::cout << "Threshold must be in (0, 1]" << std::endl;
        return 1;
    }
    if (!exists(directory1)) {
        std::cout << "Directory " << directory1 << " does not exist" << std::endl;
        return 1;
    }
    if (!exists(directory2)) {
        std::cout << "Directory " << directory2 << " does not exist" << std::endl;
        return 1;
    }

    compare_files(directory1, directory2, threshold);
}