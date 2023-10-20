#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace std::filesystem;

const int CNT_BIG_EQUAL = 1;
const int BIG_SIZE_MB = 10;
const int CNT_BIG_DIFFERENT = 1;

const int CNT_SMALL = 1;
const int SMALL_SIZE_KB = 1;

const int CNT_SIMILAR = 1;
const int SIMILAR_SIZE_KB = 5;

const char* dir1 = "dir1";
const char* dir2 = "dir2";


int main() {
    path p1(dir1);
    path p2(dir2);

    create_directory(p1);
    create_directory(p2);

    std::vector<unsigned char> equal_buffer(BIG_SIZE_MB * 1024 * 1024);
    for (auto& el: equal_buffer) {
        el = rand() % 256;
    }

    for (int i = 0; i < CNT_BIG_EQUAL; ++i) {
        std::ofstream file1(p1 / ("equal" + std::to_string(i)), std::ios::binary);
        std::ofstream file2(p2 / ("equal" + std::to_string(i)), std::ios::binary);
        file1.write(reinterpret_cast<char*>(equal_buffer.data()), equal_buffer.size());
        file2.write(reinterpret_cast<char*>(equal_buffer.data()), equal_buffer.size());
        file1.close();
        file2.close();
    }

    for (int i = 0; i < CNT_BIG_DIFFERENT; ++i) {
        std::vector<unsigned char> buffer1(BIG_SIZE_MB * 1024 * 1024);
        std::vector<unsigned char> buffer2(BIG_SIZE_MB * 1024 * 1024);
        for (auto& el: buffer1) {
            el = rand() % 256;
        }
        for (auto& el: buffer2) {
            el = rand() % 256;
        }
        std::ofstream file1(p1 / ("different" + std::to_string(i)), std::ios::binary);
        std::ofstream file2(p2 / ("different" + std::to_string(i)), std::ios::binary);
        file1.write(reinterpret_cast<char*>(buffer1.data()), buffer1.size());
        file2.write(reinterpret_cast<char*>(buffer2.data()), buffer2.size());
        file1.close();
        file2.close();
    }

    for (int i = 0; i < CNT_SMALL; ++i) {
        std::vector<unsigned char> buffer1(SMALL_SIZE_KB * 1024);
        std::vector<unsigned char> buffer2(SMALL_SIZE_KB * 1024);
        for (auto& el: buffer1) {
            el = rand() % 256;
        }
        for (auto& el: buffer2) {
            el = rand() % 256;
        }

        std::ofstream file1(p1 / ("small" + std::to_string(i)), std::ios::binary);
        std::ofstream file2(p2 / ("small" + std::to_string(i)), std::ios::binary);
        file1.write(reinterpret_cast<char*>(buffer1.data()), buffer1.size());
        file2.write(reinterpret_cast<char*>(buffer2.data()), buffer2.size());
        file1.close();
        file2.close();
    }

    std::vector<unsigned char> similar_buffer(SIMILAR_SIZE_KB * 1024);
    for (auto& el: similar_buffer) {
        el = rand() % 256;
    }
    std::ofstream file1(p1 / "similar_left", std::ios::binary);
    file1.write(reinterpret_cast<char*>(similar_buffer.data()), similar_buffer.size());
    file1.close();

    for (int i = 0; i < CNT_SIMILAR; ++i) {
        auto buffer2 = similar_buffer;
        int new_symbols = rand() % 1000;
        for (int j = 0; j < new_symbols; ++j) {
            buffer2[rand() % buffer2.size()] = rand() % 256;
        }

        std::ofstream file2(p2 / ("similar_right" + std::to_string(i)), std::ios::binary);
        file2.write(reinterpret_cast<char*>(similar_buffer.data()), similar_buffer.size());
        file2.close();
    }
}
