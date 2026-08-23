#include "runtime.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <iostream>

namespace eng {
namespace runtime {

void* alloc(size_t size) {
    return malloc(size);
}

void dealloc(void* ptr) {
    free(ptr);
}

void* realloc(void* ptr, size_t newSize) {
    return ::realloc(ptr, newSize);
}

std::string runtimeString(const char* s) {
    return std::string(s ? s : "");
}

std::string runtimeConcat(const std::string& a, const std::string& b) {
    return a + b;
}

int runtimeLength(const std::string& s) {
    return (int)s.length();
}

void runtimePrint(const std::string& s) {
    std::cout << s;
}

void runtimePrintln(const std::string& s) {
    std::cout << s << "\n";
}

void runtimePrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// --- Vector implementation ---
struct VecData {
    std::vector<void*> items;
};

void* vectorCreate() {
    return new VecData();
}

void vectorPush(void* vec, void* item) {
    if (vec) static_cast<VecData*>(vec)->items.push_back(item);
}

void vectorPop(void* vec) {
    if (vec && !static_cast<VecData*>(vec)->items.empty()) {
        static_cast<VecData*>(vec)->items.pop_back();
    }
}

int vectorSize(void* vec) {
    if (!vec) return 0;
    return (int)static_cast<VecData*>(vec)->items.size();
}

void* vectorGet(void* vec, int index) {
    if (!vec || index < 0) return nullptr;
    auto* v = static_cast<VecData*>(vec);
    if (index >= (int)v->items.size()) return nullptr;
    return v->items[index];
}

void vectorDestroy(void* vec) {
    delete static_cast<VecData*>(vec);
}

// --- Map implementation ---
struct MapData {
    std::map<std::string, void*> items;
};

void* mapCreate() {
    return new MapData();
}

void mapPut(void* map, const std::string& key, void* value) {
    if (map) static_cast<MapData*>(map)->items[key] = value;
}

void* mapGet(void* map, const std::string& key) {
    if (!map) return nullptr;
    auto it = static_cast<MapData*>(map)->items.find(key);
    return it != static_cast<MapData*>(map)->items.end() ? it->second : nullptr;
}

bool mapContains(void* map, const std::string& key) {
    if (!map) return false;
    return static_cast<MapData*>(map)->items.find(key) != static_cast<MapData*>(map)->items.end();
}

void mapRemove(void* map, const std::string& key) {
    if (map) static_cast<MapData*>(map)->items.erase(key);
}

int mapSize(void* map) {
    if (!map) return 0;
    return (int)static_cast<MapData*>(map)->items.size();
}

void mapDestroy(void* map) {
    delete static_cast<MapData*>(map);
}

} // namespace runtime
} // namespace eng
