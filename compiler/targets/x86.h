// EnginotechC++ — x86_64 Backend (host/debug)
// Existing backend — kept for compatibility

#ifndef ENG_TARGET_X86_H
#define ENG_TARGET_X86_H

#include "target.h"
#include <string>
#include <vector>

namespace eng {
namespace target {

class X86Backend : public TargetBackend {
public:
    Arch arch() const override { return Arch::X86_64; }
    std::vector<std::string> compileFlags(const TargetSpec& spec) const override;
    std::vector<std::string> linkFlags(const TargetSpec& spec) const override;
    std::string generateStartup(const TargetSpec& spec) const override;
    bool linkFirmware(const std::string& objPath,
                      const std::string& outPath,
                      const TargetSpec& spec) const override;
    bool upload(const std::string& firmwarePath,
                const BoardDef& board,
                const std::string& port) const override;
    std::vector<BoardDef> detectBoards() const override;
};

} // namespace target
} // namespace eng

#endif // ENG_TARGET_X86_H
