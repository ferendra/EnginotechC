// EnginotechC++ — Storage Implementation
#include "storage.h"
#include <cstring>
#include <cstdio>
#include <map>
#include <sstream>
#include <algorithm>

namespace eng {
namespace storage {

// Use a simple in-memory map as fallback
static std::map<std::string, std::string> kv_store;

Storage open(const char* backend) {
    Storage s{};
    if (backend && strcmp(backend, "nvs") == 0) s.backend = "nvs";
    else if (backend && strcmp(backend, "eeprom") == 0) s.backend = "eeprom";
    else if (backend && strcmp(backend, "sd") == 0) s.backend = "sd";
    else s.backend = "memory";  // Fallback
    printf("[Storage] Opened backend: %s\n", s.backend.c_str());
    return s;
}

void close(Storage&) {}

bool write(Storage&, const char* key, const char* value) {
    kv_store[key] = value;
    return true;
}

bool writeInt(Storage&, const char* key, int value) {
    kv_store[key] = std::to_string(value);
    return true;
}

bool writeFloat(Storage&, const char* key, float value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.4f", value);
    kv_store[key] = buf;
    return true;
}

std::string read(Storage&, const char* key) {
    auto it = kv_store.find(key);
    return (it != kv_store.end()) ? it->second : "";
}

int readInt(Storage& s, const char* key, int default_val) {
    std::string v = read(s, key);
    if (v.empty()) return default_val;
    return std::stoi(v);
}

float readFloat(Storage& s, const char* key, float default_val) {
    std::string v = read(s, key);
    if (v.empty()) return default_val;
    return std::stof(v);
}

bool erase(Storage&, const char* key) {
    kv_store.erase(key);
    return true;
}

bool eraseAll(Storage&) {
    kv_store.clear();
    return true;
}

bool contains(Storage& s, const char* key) {
    return kv_store.find(key) != kv_store.end();
}

std::vector<std::string> keys(Storage&) {
    std::vector<std::string> result;
    for (const auto& [k, _] : kv_store) result.push_back(k);
    return result;
}

// File storage (SD card simulation)
static std::map<std::string, std::string> file_store;

bool writeFile(Storage&, const char* path, const char* data) {
    file_store[path] = data;
    return true;
}

std::string readFile(Storage&, const char* path) {
    auto it = file_store.find(path);
    return (it != file_store.end()) ? it->second : "";
}

bool deleteFile(Storage&, const char* path) {
    return file_store.erase(path) > 0;
}

bool exists(Storage&, const char* path) {
    return file_store.find(path) != file_store.end();
}

std::vector<std::string> listFiles(Storage&, const char* dir) {
    (void)dir;
    std::vector<std::string> result;
    for (const auto& [p, _] : file_store) result.push_back(p);
    return result;
}

size_t totalBytes(Storage&) { return 4096; }  // EEPROM size
size_t usedBytes(Storage&) {
    size_t total = 0;
    for (const auto& [k, v] : kv_store) total += k.size() + v.size();
    return total;
}

} // namespace storage
} // namespace eng
