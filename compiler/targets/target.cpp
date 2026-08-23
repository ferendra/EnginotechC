// EnginotechC++ — Target Registry Implementation

#include "target.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace eng {
namespace target {
namespace fs = std::filesystem;

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

// ---- TargetRegistry ----

void TargetRegistry::registerBackend(std::shared_ptr<TargetBackend> backend) {
    backends()[backend->arch()] = std::move(backend);
}

std::shared_ptr<TargetBackend> TargetRegistry::get(Arch arch) {
    auto& m = backends();
    auto it = m.find(arch);
    return it != m.end() ? it->second : nullptr;
}

std::shared_ptr<TargetBackend> TargetRegistry::getForBoard(const std::string& boardId) {
    auto boards = loadAllBoards();
    for (const auto& b : boards) {
        if (b.id == boardId) {
            return get(b.arch);
        }
    }
    return nullptr;
}

std::vector<Arch> TargetRegistry::listArchitectures() {
    std::vector<Arch> result;
    for (const auto& [arch, _] : backends()) {
        result.push_back(arch);
    }
    return result;
}

std::map<Arch, std::shared_ptr<TargetBackend>>& TargetRegistry::backends() {
    static std::map<Arch, std::shared_ptr<TargetBackend>> s_backends;
    return s_backends;
}

// ---- Board loading ----

std::vector<BoardDef> loadAllBoards() {
    std::vector<BoardDef> boards;
    fs::path boardsDir = fs::path(PROJECT_ROOT);
    if (boardsDir.empty() || boardsDir == ".") {
        // Try common paths relative to executable location
        boardsDir = fs::current_path() / ".." / "boards";
        if (!fs::exists(boardsDir)) {
            boardsDir = fs::current_path() / "boards";
        }
    } else {
        boardsDir /= "boards";
    }
    if (!fs::exists(boardsDir)) return boards;

    for (const auto& entry : fs::directory_iterator(boardsDir)) {
        if (!entry.is_directory()) continue;
        fs::path cfgFile = entry.path() / "board.json";
        if (!fs::exists(cfgFile)) continue;

        std::ifstream f(cfgFile);
        if (!f) continue;
        std::stringstream ss;
        ss << f.rdbuf();
        std::string content = ss.str();

        BoardDef bd{};
        bd.id = entry.path().filename().string();
        bd.name = bd.id;  // Will be overridden by JSON parsing below

        // Parse basic fields (simplified JSON parser)
        auto findVal = [&](const std::string& key) -> std::string {
            std::string search = "\"" + key + "\":\"";
            size_t pos = content.find(search);
            if (pos == std::string::npos) {
                // Try with space after colon
                search = "\"" + key + "\": \"";
                pos = content.find(search);
            }
            if (pos == std::string::npos) return "";
            pos += search.size();
            size_t end = content.find('"', pos);
            return content.substr(pos, end - pos);
        };

        auto findNum = [&](const std::string& key) -> int64_t {
            std::string search = "\"" + key + "\":";
            size_t pos = content.find(search);
            if (pos == std::string::npos) return 0;
            pos += search.size();
            while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t')) ++pos;
            std::string numStr;
            while (pos < content.size() && (isdigit(content[pos]) || content[pos] == '-')) {
                numStr += content[pos++];
            }
            return numStr.empty() ? 0 : std::stoll(numStr);
        };

        bd.name = findVal("name");
        std::string archStr = findVal("architecture");
        if (archStr == "avr") bd.arch = Arch::AVR;
        else if (archStr == "xtensa") bd.arch = Arch::XTENSA_ESP32;
        else if (archStr == "riscv") bd.arch = Arch::RISC_V_ESP32;
        else bd.arch = Arch::UNKNOWN;

        bd.cpu = findVal("cpu");
        bd.clockHz = static_cast<uint32_t>(findNum("clock_hz"));
        bd.flashBytes = static_cast<size_t>(findNum("flash_bytes"));
        bd.ramBytes = static_cast<size_t>(findNum("ram_bytes"));
        bd.builtinLedPin = static_cast<int>(findNum("builtin_led_pin"));
        bd.hasUART = findVal("has_uart") == "true";
        bd.hasSPI = findVal("has_spi") == "true";
        bd.hasI2C = findVal("has_i2c") == "true";
        bd.hasWiFi = findVal("has_wifi") == "true";
        bd.hasBLE = findVal("has_ble") == "true";
        bd.compilerPath = findVal("compiler_path");
        bd.linkerScript = findVal("linker_script");
        bd.uploadTool = findVal("upload_tool");
        bd.uploadBaud = findVal("upload_baud");

        boards.push_back(bd);
    }
    return boards;
}

std::optional<BoardDef> findBoard(const std::string& boardId) {
    auto boards = loadAllBoards();
    for (const auto& b : boards) {
        if (b.id == boardId) return b;
    }
    return std::nullopt;
}

TargetSpec buildTargetSpec(const std::string& target,
                           const std::string& boardId,
                           const std::string& runtimeProfile) {
    TargetSpec spec{};
    spec.runtimeProfile = runtimeProfile.empty() ? "embedded-min" : runtimeProfile;

    if (!boardId.empty()) {
        auto bd = findBoard(boardId);
        if (bd.has_value()) {
            spec.board = bd.value();
            return spec;
        }
    }

    // Fallback: target-based default
    if (target == "arduino" || target == "avr") {
        spec.board.id = "arduino-uno";
        spec.board.name = "Arduino Uno";
        spec.board.arch = Arch::AVR;
        spec.board.cpu = "ATmega328P";
        spec.board.clockHz = 16000000;
        spec.board.flashBytes = 32768;
        spec.board.ramBytes = 2048;
        spec.board.builtinLedPin = 13;
        spec.board.hasUART = true;
        spec.board.hasSPI = true;
        spec.board.hasI2C = true;
    } else if (target == "esp32") {
        spec.board.id = "esp32-devkit";
        spec.board.name = "ESP32 DevKit";
        spec.board.arch = Arch::XTENSA_ESP32;
        spec.board.cpu = "ESP32-D0WDQ6";
        spec.board.clockHz = 240000000;
        spec.board.flashBytes = 4 * 1024 * 1024;
        spec.board.ramBytes = 524288;
        spec.board.builtinLedPin = 2;
        spec.board.hasUART = true;
        spec.board.hasSPI = true;
        spec.board.hasI2C = true;
        spec.board.hasWiFi = true;
        spec.board.hasBLE = true;
    } else {
        spec.board.id = "unknown";
        spec.board.arch = Arch::UNKNOWN;
    }

    return spec;
}

} // namespace target
} // namespace eng
