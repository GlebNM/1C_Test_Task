#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

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

    bool similar(const file_data& other, const float threshold, float& similarity) const {
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

        for (auto it = buffer.begin(); it != buffer.end(); ++it) {
            auto pos = std::find(other_buffer.begin(), other_buffer.end(), *it);
            if (pos == other_buffer.end()) {
                return false;
            }
            int count_similar = 0;
            while (pos != other_buffer.end() && it != buffer.end()) {
                while (*pos == *it) {
                    ++count_similar;
                    ++pos;
                    ++it;
                    if (pos == other_buffer.end() || it == buffer.end()) {
                        break;
                    }
                }
                ++pos;
            }

        }
        similarity = static_cast<float>(size) / other_size;
        return similarity >= threshold;
    }
};

struct similar_files_data {
    path path1;
    path path2;
    float similarity;
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
    std::vector<similar_files_data> similar_files;
    for (auto& file1: files1) {
        for (auto& file2: files2) {
            if (file1.second.hash == file2.second.hash && file1.second.size == file2.second.size) {
                if (file1.second.equals(file2.second)) {
                    equal_files.emplace_back(file1.second.path, file2.second.path);
                    file1.second.is_similar_or_equal = true;
                    file2.second.is_similar_or_equal = true;
                }
            } else {
                bool are_similar = false;
                float similarity;
                if (file1.second.size <= file2.second.size) {
                    are_similar = file1.second.similar(file2.second, threshold, similarity);
                } else {
                    are_similar = file2.second.similar(file1.second, threshold, similarity);
                }
                if (are_similar) {
                    similar_files.emplace_back(
                            similar_files_data{file1.second.path, file2.second.path, similarity});
                    file1.second.is_similar_or_equal = true;
                    file2.second.is_similar_or_equal = true;
                }
            }
        }
    }

    std::cout << "Equal files:" << std::endl;
    for (const auto& [path1, path2]: equal_files) {
        std::cout << '\t' << path1 << " " << path2 << std::endl;
    }

    std::cout << "Similar files:" << std::endl;
    for (const auto& [path1, path2, similarity]: similar_files) {
        std::cout << '\t' << path1 << " " << path2 << " (similarity=" << similarity << ")" << std::endl;
    }

    std::cout << "Unique files in " << directory1 << ":" << std::endl;
    for (const auto& [name, file]: files1) {
        if (!file.is_similar_or_equal) {
            std::cout << '\t' << file.path << std::endl;
        }
    }

    std::cout << "Unique files in " << directory2 << ":" << std::endl;
    for (const auto& [name, file]: files2) {
        if (!file.is_similar_or_equal) {
            std::cout << '\t' << file.path << std::endl;
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