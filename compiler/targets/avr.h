// EnginotechC++ — Arduino Target Backend
// Implements the TargetBackend interface for AVR/Arduino boards.
// Generates .ino sketches via arduinogen namespace, then invokes avr-gcc.

#pragma once
#include "target.h"
#include <string>
#include <vector>

namespace eng {
namespace target {

class AVRBackend : public TargetBackend {
public:
    Arch arch() const override { return Arch::AVR; }

    std::vector<std::string> compileFlags(const TargetSpec&) const override;
    std::vector<std::string> linkFlags(const TargetSpec&) const override;
    std::string generateStartup(const TargetSpec&) const override;
    bool linkFirmware(const std::string& objPath,
                      const std::string& outPath,
                      const TargetSpec& spec) const override;
    bool upload(const std::string& firmwarePath,
                const BoardDef& board,
                const std::string& port) const override;
    std::vector<BoardDef> detectBoards() const override;

private:
    std::string findAvrGcc() const;
    std::string findAvrdude() const;
};

} // namespace target
} // namespace eng
