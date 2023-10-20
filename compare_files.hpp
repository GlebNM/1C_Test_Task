#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <cmath>

#include <openssl/sha.h>

using namespace std::filesystem;

struct file_data {
    std::filesystem::path path;
    std::string name;
    unsigned char hash[SHA_DIGEST_LENGTH];
    size_t size = 0;
    bool is_similar_or_equal = false;

    void get_file_hash(const auto& path, unsigned char** hash) {
        unsigned long long result = 0;
        size_t size = file_size(path);
        std::vector<unsigned char> buffer(size);
        std::ifstream file(path, std::ios::binary);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        file.close();

        SHA1(buffer.data(), size, *hash);
    }

    explicit file_data(const auto& path) {
        this->path = path;
        name = path.filename().string();
        unsigned char* ptr = hash; //
        get_file_hash(path, &ptr);
        size = file_size(path);
    }

    file_data() = default;

    file_data(const file_data&) = default;

    bool equals(const file_data& other) const {
        if (size != other.size) {
            return false;
        }
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
            if (hash[i] != other.hash[i]) {
                return false;
            }
        }
        return true;
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
    std::vector<file_data> files1, files2;
    for (const auto& entry1: directory_iterator(directory1)) {
        files1.emplace_back(entry1.path());
    }
    for (const auto& entry2: directory_iterator(directory2)) {
        files2.emplace_back(entry2.path());
    }

    std::vector<std::pair<path, path>> equal_files;
    std::vector<similar_files_data> similar_files;
    for (auto& file1: files1) {
        for (auto& file2: files2) {
            if (file1.equals(file2)) {
                equal_files.emplace_back(file1.path, file2.path);
                file1.is_similar_or_equal = true;
                file2.is_similar_or_equal = true;
            }
        }
    }

    std::sort(files1.begin(), files1.end(), [](const file_data& file1, const file_data& file2) {
        return file1.size < file2.size;
    });
    std::sort(files2.begin(), files2.end(), [](const file_data& file1, const file_data& file2) {
        return file1.size < file2.size;
    });
    for (auto& file: files1) {
        size_t size1 = file.size;
        float max_next_size = size1 / threshold;
        size_t next_size = ceil(max_next_size);

        auto max_file_iter = lower_bound(files2.begin(), files2.end(), size1,
                                         [](const file_data& file, size_t size) {
                                             return file.size < size;
                                         });

        for (auto it2 = max_file_iter; it2 != files2.end(); ++it2) {
            if (it2->size > next_size) {
                break;
            }
            float similarity;
            if (file.similar(*it2, threshold, similarity)) {
                similar_files.emplace_back(
                        similar_files_data{file.path, it2->path, similarity});
                file.is_similar_or_equal = true;
                it2->is_similar_or_equal = true;
            }
        }
    }

    for (auto& file: files2) {
        size_t size2 = file.size;
        float max_next_size = size2 / threshold;
        size_t next_size = ceil(max_next_size);

        auto max_file_iter = upper_bound(files1.begin(), files1.end(), size2,
                                         [](size_t size, const file_data& file) {
                                             return size < file.size;
                                         });
        for (auto it1 = max_file_iter; it1 != files1.end(); ++it1) {
            if (it1->size > next_size) {
                break;
            }
            float similarity;
            if (file.similar(*it1, threshold, similarity)) {
                similar_files.emplace_back(
                        similar_files_data{it1->path, file.path, similarity});
                file.is_similar_or_equal = true;
                it1->is_similar_or_equal = true;
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
    for (const auto& file: files1) {
        if (!file.is_similar_or_equal) {
            std::cout << '\t' << file.path << std::endl;
        }
    }

    std::cout << "Unique files in " << directory2 << ":" << std::endl;
    for (const auto& file: files2) {
        if (!file.is_similar_or_equal) {
            std::cout << '\t' << file.path << std::endl;
        }
    }
}