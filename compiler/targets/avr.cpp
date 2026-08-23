// EnginotechC++ — AVR Backend Implementation

#include "avr.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace eng {
namespace target {
namespace fs = std::filesystem;

std::string AVRBackend::findAvrGcc() const {
    // Common Windows paths for llvm-mingw avr toolchain
    const char* candidates[] = {
        "avr-gcc",
        "C:/Program Files/LLVM/bin/avr-gcc",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        if (std::system((std::string(candidates[i]) + " --version >nul 2>&1").c_str()) == 0) {
            return candidates[i];
        }
    }
    return "";
}

std::string AVRBackend::findAvrdude() const {
    const char* candidates[] = {
        "avrdude",
        "C:/Program Files/LLVM/bin/avrdude",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        if (std::system((std::string(candidates[i]) + " --version >nul 2>&1").c_str()) == 0) {
            return candidates[i];
        }
    }
    return "";
}

std::vector<std::string> AVRBackend::compileFlags(const TargetSpec&) const {
    return {
        "-mmcu=atmega328p",
        "-Wall",
        "-std=gnu11",
        "-Os",
        "-DF_CPU=16000000UL",
        "-I.",
        "-Iruntime/embedded",
        "-Istd/embedded"
    };
}

std::vector<std::string> AVRBackend::linkFlags(const TargetSpec&) const {
    return {
        "-mmcu=atmega328p",
        "-Wl,-Map=output.map",
        "-lm"
    };
}

std::string AVRBackend::generateStartup(const TargetSpec&) const {
    // AVR startup: __cmain calls main()
    // This is handled by avr-libc; we emit a small wrapper
    return R"(
// AVR startup wrapper — emitted by EC compiler
// avr-libc __cmain calls this as main()
int main(void);
int main(void) {
    // Call the generated EC main
    return main();
}
)";
}

bool AVRBackend::linkFirmware(const std::string& objPath,
                               const std::string& outPath,
                               const TargetSpec& spec) const {
    (void)spec;
    std::string avrGcc = findAvrGcc();
    if (avrGcc.empty()) {
        std::fprintf(stderr, "Error: avr-gcc not found. Install llvm-mingw-avr.\n");
        return false;
    }

    std::string cmd = avrGcc
        + " -mmcu=atmega328p -Os -o \"" + outPath + "\" \"" + objPath + "\" -lm";
    return std::system(cmd.c_str()) == 0;
}

bool AVRBackend::upload(const std::string& firmwarePath,
                         const BoardDef& board,
                         const std::string& port) const {
    (void)board;
    std::string avrdude = findAvrdude();
    if (avrdude.empty()) {
        std::fprintf(stderr, "Error: avrdude not found. Cannot flash board.\n");
        return false;
    }

    std::string stk500 = (port.empty() || port == "auto")
        ? "-c arduino"
        : "-c arduino";

    std::string portArg = port.empty() ? "/dev/ttyUSB0" : port;
    // On Windows, avrdude uses COM ports
    if (port.find("COM") == 0) {
        portArg = port;
    }

    std::string cmd = avrdude
        + " -c " + stk500
        + " -p atmega328p"
        + " -P " + portArg
        + " -b 115200"
        + " -U flash:w:\""+ firmwarePath + "\":i";
    return std::system(cmd.c_str()) == 0;
}

std::vector<BoardDef> AVRBackend::detectBoards() const {
    // Enumerate USB serial ports and check for known AVR signatures
    std::vector<BoardDef> result;
    // In production: scan /dev/tty* or COM ports, check vendor/product IDs
    (void)result;
    return result;
}

} // namespace target
} // namespace eng
