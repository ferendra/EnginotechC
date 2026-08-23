// EnginotechC++ — x86_64 Backend Implementation (host/debug)

#include "x86.h"
#include <cstdio>
#include <cstdlib>
#include <filesystem>

namespace eng {
namespace target {
namespace fs = std::filesystem;

std::vector<std::string> X86Backend::compileFlags(const TargetSpec&) const {
    return {"-O2", "-target", "x86_64-pc-windows-msvc", "-c"};
}

std::vector<std::string> X86Backend::linkFlags(const TargetSpec&) const {
    return {};
}

std::string X86Backend::generateStartup(const TargetSpec&) const {
    return "";  // x86 main is entry point
}

bool X86Backend::linkFirmware(const std::string& objPath,
                               const std::string& outPath,
                               const TargetSpec&) const {
    (void)objPath;
    (void)outPath;
    // x86 compilation is handled by LLVMCodeGen directly in the existing pipeline
    return true;
}

bool X86Backend::upload(const std::string&, const BoardDef&, const std::string&) const {
    return false;  // N/A for host
}

std::vector<BoardDef> X86Backend::detectBoards() const {
    return {};
}

} // namespace target
} // namespace eng
