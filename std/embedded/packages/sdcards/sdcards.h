// EnginotechC++ — SD Card Filesystem Package (SPI)

#ifndef ENG_SDCARDS_H
#define ENG_SDCARDS_H

#include <string>
#include <vector>
#include <cstdint>

namespace eng {
namespace sdcards {

struct SDCard {
    int spi_bus;
    int pin_cs;
    bool mounted;
    uint64_t size_mb;
    uint64_t free_mb;
};

SDCard open(int spi_bus, int pin_cs);
void close(SDCard& card);
bool isMounted(SDCard& card);

// File operations
bool writeFile(SDCard& card, const char* path, const char* data);
std::string readFile(SDCard& card, const char* path);
bool appendFile(SDCard& card, const char* path, const char* data);
bool deleteFile(SDCard& card, const char* path);
bool exists(SDCard& card, const char* path);

// Directory operations
bool mkdir(SDCard& card, const char* path);
bool rmdir(SDCard& card, const char* path);
std::vector<std::string> listDir(SDCard& card, const char* path);

// Card info
uint64_t getTotalBytes(SDCard& card);
uint64_t getUsedBytes(SDCard& card);
const char* getCardType(SDCard& card);  // "SDHC", "SDXC", "MMC", etc.

} // namespace sdcards
} // namespace eng

#endif // ENG_SDCARDS_H
