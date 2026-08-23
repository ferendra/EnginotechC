// EnginotechC++ — SD Card Implementation
#include "sdcards.h"
#include <cstdio>
#include <map>
#include <cstdint>

namespace eng {
namespace sdcards {

static std::map<std::string, std::string> fs;
static std::map<int, SDCard> cards;
static int next_card_id = 0;

SDCard open(int spi_bus, int pin_cs) {
    int id = next_card_id++;
    SDCard c{};
    c.spi_bus = spi_bus;
    c.pin_cs = pin_cs;
    c.mounted = true;
    cards[id] = c;
    return c;
}

void close(SDCard& card) { card.mounted = false; }
bool isMounted(SDCard& card) { return card.mounted; }

bool writeFile(SDCard& card, const char* path, const char* data) {
    auto key = std::to_string((uintptr_t)&card) + path;
    fs[key] = data;
    return true;
}

std::string readFile(SDCard& card, const char* path) {
    (void)card;
    auto it = fs.find(path);
    return it != fs.end() ? it->second : "";
}

bool appendFile(SDCard& card, const char* path, const char* data) {
    fs[path] += data;
    return true;
}

bool deleteFile(SDCard&, const char* path) { return fs.erase(path) > 0; }
bool exists(SDCard&, const char* path) { return fs.count(path) > 0; }

bool mkdir(SDCard&, const char*) { return true; }
bool rmdir(SDCard&, const char*) { return true; }

std::vector<std::string> listDir(SDCard&, const char* dir) {
    (void)dir;
    std::vector<std::string> result;
    for (const auto& [p, _] : fs) result.push_back(p);
    return result;
}

uint64_t getTotalBytes(SDCard&) { return 1024ULL * 1024 * 1024; }
uint64_t getUsedBytes(SDCard&) { return 200ULL * 1024 * 1024; }
const char* getCardType(SDCard&) { return "SDHC"; }

} // namespace sdcards
} // namespace eng
