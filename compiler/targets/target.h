// EnginotechC++ — Hardware Target Backend Interface
// Abstract base for all target backends (Arduino AVR, ESP32 Xtensa/RISC-V, etc.)

#ifndef ENG_TARGET_H
#define ENG_TARGET_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <optional>

namespace eng {
namespace target {

enum class Arch { AVR, XTENSA_ESP32, RISC_V_ESP32, X86_64, UNKNOWN };
enum class OS { EMBEDDED, LINUX, WINDOWS, MACOS };

// A single board definition
struct BoardDef {
    std::string id;                  // e.g. "arduino-uno", "esp32-devkit"
    std::string name;                // Human-readable name
    Arch arch;
    OS os;
    std::string cpu;                 // e.g. "ATmega328P", "ESP32-D0WDQ6"
    uint32_t clockHz;                // e.g. 16000000
    size_t flashBytes;               // e.g. 32768
    size_t ramBytes;                 // e.g. 2048
    int builtinLedPin = -1;          // -1 if none

    // GPIO pin map
    std::map<std::string, int> gpioPins;      // name -> pin number
    std::map<std::string, int> pwmPins;       // name -> pin number
    std::map<std::string, int> adcPins;       // name -> pin number

    // Peripherals
    bool hasUART = false;
    bool hasSPI = false;
    bool hasI2C = false;
    bool hasWiFi = false;
    bool hasBLE = false;

    // Toolchain paths (populated by board config JSON)
    std::string compilerPath;
    std::string linkerScript;
    std::string uploadTool;
    std::string uploadBaud;

    // Linker flags
    std::vector<std::string> linkFlags;
    std::vector<std::string> compileFlags;
};

// The active compilation target
struct TargetSpec {
    BoardDef board;
    std::string optimization = "O2";
    bool debug = false;
    std::string runtimeProfile = "embedded-min";  // "embedded-min" or "embedded-full"
};

// Backend interface: all targets implement this
class TargetBackend {
public:
    virtual ~TargetBackend() = default;

    // Return the target architecture
    virtual Arch arch() const = 0;

    // Generate target-specific compile flags
    virtual std::vector<std::string> compileFlags(const TargetSpec& spec) const = 0;

    // Generate target-specific linker flags
    virtual std::vector<std::string> linkFlags(const TargetSpec& spec) const = 0;

    // Generate startup code / main wrapper
    virtual std::string generateStartup(const TargetSpec& spec) const = 0;

    // Compile IR object file to firmware image
    virtual bool linkFirmware(const std::string& objPath,
                              const std::string& outPath,
                              const TargetSpec& spec) const = 0;

    // Upload firmware to board (returns true on success)
    virtual bool upload(const std::string& firmwarePath,
                        const BoardDef& board,
                        const std::string& port = "") const = 0;

    // Detect connected boards of this type
    virtual std::vector<BoardDef> detectBoards() const = 0;
};

// Registry of all known backends
class TargetRegistry {
public:
    // Register a backend for an architecture
    static void registerBackend(std::shared_ptr<TargetBackend> backend);

    // Get backend for architecture, or nullptr
    static std::shared_ptr<TargetBackend> get(Arch arch);

    // Get backend for board ID string
    static std::shared_ptr<TargetBackend> getForBoard(const std::string& boardId);

    // List all registered architectures
    static std::vector<Arch> listArchitectures();

private:
    static std::map<Arch, std::shared_ptr<TargetBackend>>& backends();
};

// Load board definitions from boards/ directory
std::vector<BoardDef> loadAllBoards();

// Find a board by ID
std::optional<BoardDef> findBoard(const std::string& boardId);

// Build a TargetSpec from CLI args
TargetSpec buildTargetSpec(const std::string& target,
                           const std::string& boardId,
                           const std::string& runtimeProfile);

} // namespace target
} // namespace eng

#endif // ENG_TARGET_H
