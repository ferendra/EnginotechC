// EnginotechC++ — Storage Package
// Unified storage API: NVS (ESP32), EEPROM (Arduino), SD card

#ifndef ENG_STORAGE_H
#define ENG_STORAGE_H

#include <string>
#include <vector>

namespace eng {
namespace storage {

// Open a storage backend
// For ESP32: uses NVS (Non-Volatile Storage)
// For Arduino: uses EEPROM
// For ESP32/Arduino with SD: uses SD card
struct Storage {
    std::string backend;   // "nvs", "eeprom", "sd"
    int partition;         // NVS partition index (ESP32)
    int pin_cs;            // SD card CS pin
};

Storage open(const char* backend = "auto");
void close(Storage& s);

// Key-value store
bool write(Storage& s, const char* key, const char* value);
bool writeInt(Storage& s, const char* key, int value);
bool writeFloat(Storage& s, const char* key, float value);
std::string read(Storage& s, const char* key);
int readInt(Storage& s, const char* key, int default_val = 0);
float readFloat(Storage& s, const char* key, float default_val = 0.0f);
bool erase(Storage& s, const char* key);
bool eraseAll(Storage& s);
bool contains(Storage& s, const char* key);
std::vector<std::string> keys(Storage& s);

// File-style storage (SD card only)
bool writeFile(Storage& s, const char* path, const char* data);
std::string readFile(Storage& s, const char* path);
bool deleteFile(Storage& s, const char* path);
bool exists(Storage& s, const char* path);
std::vector<std::string> listFiles(Storage& s, const char* dir = "/");

// Size info
size_t totalBytes(Storage& s);
size_t usedBytes(Storage& s);

} // namespace storage
} // namespace eng

#endif // ENG_STORAGE_H
