// EnginotechC++ — Bare-metal x86_64 Target Backend
// For building OS kernels and firmware without any OS dependency.
// Produces ELF64 executable with custom entry point, or raw binary via objcopy.

#pragma once
#include "target.h"
#include <string>
#include <vector>

namespace eng {
namespace target {

// Bare-metal x86_64 backend: no OS runtime, custom memory layout, raw binary output.
class BareMetalBackend : public TargetBackend {
public:
    Arch arch() const override { return Arch::X86_64; }

    std::vector<std::string> compileFlags(const TargetSpec&) const override;
    std::vector<std::string> linkFlags(const TargetSpec& spec) const override;
    std::string generateStartup(const TargetSpec& spec) const override;
    bool linkFirmware(const std::string& objPath,
                      const std::string& outPath,
                      const TargetSpec& spec) const override;
    bool upload(const std::string&, const BoardDef&, const std::string&) const override;
    std::vector<BoardDef> detectBoards() const override;

    // Extract raw binary from ELF using llvm-objcopy
    static bool extractBinary(const std::string& elfPath,
                              const std::string& binPath);

    // Find llvm-objcopy in common locations
    static std::string findObjcopy();

    // Generate a linker script (internal helper, exposed for embedded.cpp)
    static std::string generateLinkerScriptFor(const BoardDef& board);
};

} // namespace target
} // namespace eng
