// Runtime library for EnginotechC++
#ifndef ENG_RUNTIME_H
#define ENG_RUNTIME_H

#include <string>
#include <vector>
#include <map>

namespace eng {
namespace runtime {

// Memory management utilities
void* alloc(size_t size);
void dealloc(void* ptr);
void* realloc(void* ptr, size_t newSize);

// String utilities
std::string runtimeString(const char* s);
std::string runtimeConcat(const std::string& a, const std::string& b);
int runtimeLength(const std::string& s);

// Output utilities
void runtimePrint(const std::string& s);
void runtimePrintln(const std::string& s);
void runtimePrintf(const char* format, ...);

// Vector utilities (for the runtime vector type)
void* vectorCreate();
void vectorPush(void* vec, void* item);
void vectorPop(void* vec);
int vectorSize(void* vec);
void* vectorGet(void* vec, int index);
void vectorDestroy(void* vec);

// Map utilities
void* mapCreate();
void mapPut(void* map, const std::string& key, void* value);
void* mapGet(void* map, const std::string& key);
bool mapContains(void* map, const std::string& key);
void mapRemove(void* map, const std::string& key);
int mapSize(void* map);
void mapDestroy(void* map);

} // namespace runtime
} // namespace eng

#endif // ENG_RUNTIME_H
