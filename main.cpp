#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>
#include <cassert>
#include <openssl/sha.h>

using namespace std::filesystem;

struct file_data {
    std::filesystem::path path;
    std::string name;
    unsigned long long hash = 0;
    size_t size = 0;
    bool is_similar_or_equal = false;

    unsigned long long get_file_hash(const auto& path) {
        unsigned long long result = 0;
        size_t size = file_size(path);
        std::vector<unsigned char> buffer(size);
        std::ifstream file(path, std::ios::binary);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        file.close();

        SHA_CTX ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, buffer.data(), size);
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1_Final(hash, &ctx);
        for (unsigned char c: hash) {
            result += c;
        }
        return result;
    }

    explicit file_data(const auto& path) {
        this->path = path;
        name = path.filename().string();
        hash = get_file_hash(path);
        size = file_size(path);
    }

    file_data() = default;

    file_data(const file_data&) = default;

    bool equals(const file_data& other) const {
        if (hash != other.hash || size != other.size) {
            return false;
        }
        std::vector<unsigned char> buffer = read_file();
        std::vector<unsigned char> other_buffer = other.read_file();
        return buffer == other_buffer;
    }

    std::vector<unsigned char> read_file() const {
        std::vector<unsigned char> result(size);
        std::ifstream file(path, std::ios::binary);
        file.read(reinterpret_cast<char*>(result.data()), size);
        file.close();
        return result;
    }

    bool similar(const file_data& other, const float threshold) const {
        size_t other_size = other.size;
        assert(this->size <= other_size);
        if (other_size == 0) {
            return false;
        }
        if (static_cast<float>(size) / other_size < threshold) {
            return false;
        }
        std::vector<unsigned char> buffer = read_file();
        std::vector<unsigned char> other_buffer = other.read_file();
    }
};


void compare_files(const std::string& directory1, const std::string& directory2, const float threshold) {
    std::map<std::string, file_data> files1, files2;

    for (const auto& entry1: directory_iterator(directory1)) {
        files1[entry1.path().filename().string()] = file_data(entry1.path());
    }
    for (const auto& entry2: directory_iterator(directory2)) {
        files2[entry2.path().filename().string()] = file_data(entry2.path());
    }

    std::vector<std::pair<path, path>> equal_files;

    for (const auto& file1: files1) {
        for (const auto& file2: files2) {
            if (file1.second.hash == file2.second.hash && file1.second.size == file2.second.size) {
                if (file1.second.equals(file2.second)) {
                    equal_files.emplace_back(file1.second.path, file2.second.path);
                }
            }
        }
    }
}

int main() {
    std::string directory1, directory2;
    std::cout << "Enter directory 1: ";
    std::cin >> directory1;
    std::cout << "Enter directory 2: ";
    std::cin >> directory2;
    float threshold;
    std::cout << "Enter threshold for similar files (float number from 0 to 1): ";
    std::cin >> threshold;

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