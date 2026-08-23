#include "fs.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace eng {
namespace fs {
namespace fs_impl = std::filesystem;

bool exists(const std::string& path) {
    return fs_impl::exists(path);
}

bool isFile(const std::string& path) {
    return fs_impl::is_regular_file(path);
}

bool isDir(const std::string& path) {
    return fs_impl::is_directory(path);
}

std::vector<std::string> listDir(const std::string& path) {
    std::vector<std::string> result;
    if (fs_impl::is_directory(path)) {
        for (const auto& entry : fs_impl::directory_iterator(path)) {
            result.push_back(entry.path().string());
        }
    }
    return result;
}

std::string readText(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::vector<char> readBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    return std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

bool writeText(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file) return false;
    file << content;
    return true;
}

bool writeBinary(const std::string& path, const std::vector<char>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;
    file.write(data.data(), data.size());
    return true;
}

bool mkdir(const std::string& path) {
    return fs_impl::create_directories(path);
}

bool deleteFile(const std::string& path) {
    return fs_impl::remove(path) > 0;
}

bool deleteDir(const std::string& path) {
    return fs_impl::remove_all(path) > 0;
}

std::string getParent(const std::string& path) {
    return fs_impl::path(path).parent_path().string();
}

std::string getExtension(const std::string& path) {
    return fs_impl::path(path).extension().string();
}

std::string getFilename(const std::string& path) {
    return fs_impl::path(path).filename().string();
}

long long getFileSize(const std::string& path) {
    if (!fs_impl::exists(path)) return -1;
    return (long long)fs_impl::file_size(path);
}

} // namespace fs
} // namespace eng
