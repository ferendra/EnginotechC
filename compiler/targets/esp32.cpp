// EnginotechC++ — ESP32 Backend Implementation

#include "esp32.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace eng {
namespace target {
namespace fs = std::filesystem;

Arch ESP32Backend::arch() const { return Arch::XTENSA_ESP32; }

std::string ESP32Backend::findEsptool() const {
    const char* candidates[] = {
        "esptool.py",
        "esptool",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        if (std::system((std::string(candidates[i]) + " --version >nul 2>&1").c_str()) == 0) {
            return candidates[i];
        }
    }
    return "";
}

std::string ESP32Backend::findEspGcc() const {
    // Look for xtensa-esp32-elf-gcc or esp32-elf-gcc
    const char* candidates[] = {
        "xtensa-esp32-elf-gcc",
        "esp32-elf-gcc",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        if (std::system((std::string(candidates[i]) + " --version >nul 2>&1").c_str()) == 0) {
            return candidates[i];
        }
    }
    return "";
}

std::string ESP32Backend::getTargetTriple(const BoardDef& board) const {
    if (board.arch == Arch::RISC_V_ESP32) return "riscv32-esp-elf";
    return "xtensa-esp32-elf";
}

std::vector<std::string> ESP32Backend::compileFlags(const TargetSpec& spec) const {
    (void)spec;
    std::vector<std::string> flags;
    flags.push_back("-mlongcalls");
    flags.push_back("-Wno-frame-address");
    flags.push_back("-ffunction-sections");
    flags.push_back("-fdata-sections");
    flags.push_back("-Wall");
    flags.push_back("-DESP32");
    flags.push_back("-DBOARD_ID=\\\"" + spec.board.id + "\\\"");
    return flags;
}

std::vector<std::string> ESP32Backend::linkFlags(const TargetSpec& spec) const {
    (void)spec;
    return {
        "--include", "elf",
        "-Wl,-- gc-sections",
        "-Wl,--no-gc-sections"
    };
}

std::string ESP32Backend::generateStartup(const TargetSpec&) const {
    // ESP32 startup is handled by ESP-IDF / esp-hal;
    // we just need to declare our C main entry
    return R"(
// ESP32 startup — EC compiler emits this as entry point
// Real platform calls sdkconfig_init() before main()
int main(void);
int main(void) {
    return main();
}
)";
}

bool ESP32Backend::linkFirmware(const std::string& objPath,
                                 const std::string& outPath,
                                 const TargetSpec& spec) const {
    (void)spec;
    std::string gcc = findEspGcc();
    if (gcc.empty()) {
        std::fprintf(stderr, "Error: ESP32 toolchain not found. "
                             "Install esp32-xtensa-elf-gcc or esp-riscv-elf-gcc.\n");
        return false;
    }

    // For M0: we link the LLVM IR object directly with xtensa-elf-ld
    // In production, use ESP-IDF cmake build system
    std::string cmd = gcc
        + " -Wl,-Map=\"" + outPath + ".map\""
        + " -Wl,--gc-sections"
        + " -o \"" + outPath + "\" \"" + objPath + "\"";
    return std::system(cmd.c_str()) == 0;
}

bool ESP32Backend::upload(const std::string& firmwarePath,
                           const BoardDef& board,
                           const std::string& port) const {
    (void)board;
    std::string esptool = findEsptool();
    if (esptool.empty()) {
        std::fprintf(stderr, "Error: esptool.py not found. "
                             "Install esptool via: pip install esptool\n");
        return false;
    }

    std::string portArg = port.empty() ? "auto" : port;
    std::string cmd = "python \"" + esptool + "\""
        + " --chip esp32"
        + " port " + portArg
        + " baud 115200"
        + " write_flash -z 0x1000 \"" + firmwarePath + "\"";
    return std::system(cmd.c_str()) == 0;
}

std::vector<BoardDef> ESP32Backend::detectBoards() const {
    std::vector<BoardDef> boards;
    // Enumerate serial ports and match against ESP32 VID/PID
    // In production: use esptool detect or read serial port names
    return boards;
}

} // namespace target
} // namespace eng
