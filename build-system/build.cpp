#include "build.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

namespace eng {
namespace build {
namespace fs = std::filesystem;

bool BuildSystem::configure(const BuildConfig& config) {
    std::string cmakePath = findCMake();
    if (cmakePath.empty()) {
        std::cerr << "Error: cmake not found\n";
        return false;
    }

    std::string archFlag = "-DCMAKE_SYSTEM_PROCESSOR=" + config.targetArch;
    std::string buildTypeFlag = "-DCMAKE_BUILD_TYPE=" + config.buildType;
    
    std::vector<std::string> args = {
        "-B", config.outputDir,
        archFlag,
        buildTypeFlag
    };

    return runCommand(cmakePath, args);
}

bool BuildSystem::build(const BuildConfig& config) {
    std::string cmakePath = findCMake();
    if (cmakePath.empty()) return false;
    
    std::string buildType = config.buildType.empty() ? "Release" : config.buildType;
    return runCommand(cmakePath, {"--build", config.outputDir, "--config", buildType});
}

bool BuildSystem::install(const BuildConfig& config, const std::string& prefix) {
    std::string cmakePath = findCMake();
    if (cmakePath.empty()) return false;
    
    std::string prefixFlag = "--prefix=" + prefix;
    return runCommand(cmakePath, {"--install", config.outputDir, prefixFlag});
}

std::string BuildSystem::detectCompiler() {
    // Try common compilers
    const char* compilers[] = {"clang++", "g++", "c++", nullptr};
    for (int i = 0; compilers[i]; ++i) {
        std::string cmd = compilers[i] + std::string(" --version");
        if (std::system(cmd.c_str()) == 0) {
            return compilers[i];
        }
    }
    return "";
}

std::vector<std::string> BuildSystem::getInstalledCompilers() {
    std::vector<std::string> result;
    const char* compilers[] = {"clang++", "g++", "c++", "cl.exe", nullptr};
    for (int i = 0; compilers[i]; ++i) {
        std::string path = compilers[i];
        // Simple check - in real implementation would use which/find
        result.push_back(path);
    }
    return result;
}

bool BuildSystem::runCommand(const std::string& cmd, const std::vector<std::string>& args) {
    std::string fullCmd = cmd;
    for (const auto& arg : args) {
        fullCmd += " " + arg;
    }
    return std::system(fullCmd.c_str()) == 0;
}

std::string BuildSystem::findCMake() {
    const char* paths[] = {"cmake", nullptr};
    for (int i = 0; paths[i]; ++i) {
        std::string test = paths[i];
        if (std::system((test + " --version >nul 2>&1").c_str()) == 0) {
            return test;
        }
    }
    return "";
}

std::string BuildSystem::findLLVMClang() {
    // Search common paths
    const char* paths[] = {
        "clang",
        "llvm-mingw/bin/clang",
        nullptr
    };
    for (int i = 0; paths[i]; ++i) {
        if (std::system((std::string(paths[i]) + " --version >nul 2>&1").c_str()) == 0) {
            return paths[i];
        }
    }
    return "";
}

} // namespace build
} // namespace eng
