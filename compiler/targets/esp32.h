// EnginotechC++ — ESP32 Backend (Xtensa / RISC-V)
// Generates Xtensa/RISC-V GCC compatible code

#ifndef ENG_TARGET_ESP32_H
#define ENG_TARGET_ESP32_H

#include "target.h"
#include <string>
#include <vector>

namespace eng {
namespace target {

class ESP32Backend : public TargetBackend {
public:
    Arch arch() const override;
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

private:
    std::string findEsptool() const;
    std::string findEspGcc() const;
    std::string getTargetTriple(const BoardDef& board) const;
};

} // namespace target
} // namespace eng

#endif // ENG_TARGET_ESP32_H
