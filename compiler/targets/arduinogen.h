// EnginotechC++ — Arduino C++ Sketch Emitter
// Transpiles a parsed EC program into an Arduino-compatible .ino sketch.
//
// File layout (one responsibility per file):
//   shim.h / shim.cpp         — runtime bridge to Arduino core APIs
//   expr_emitter.h/.cpp       — expression emission (expr → Arduino text)
//   stmt_emitter.h/.cpp       — statement emission (stmt → Arduino text)
//   sketch_builder.h/.cpp     — top-level orchestration (full .ino output)
//
// Supported EC features:
//   - Functions (free, setup, loop, main→ignored)
//   - let / mut variable declarations
//   - if / else
//   - while loops
//   - for-in range loops (a..b, a..=b)
//   - match expressions (compiled to if-else chains)
//   - break / continue
//   - Return statements
//   - Numeric literals (int, float)
//   - String literals
//   - Boolean literals
//   - Binary arithmetic and comparison operators
//   - Unary minus / logical not
//   - Field access (e.g. led.high())
//   - Free function calls (print, println, delay, millis, micros, str)
//   - gpio.output / gpio.input / gpio.pwm
//   - uart.begin / uart.write / uart.println
//   - analog.read / analog.write
//   - System calls via system.delay / system.millis / system.micros

#ifndef ENG_TARGET_ARDUINOGEN_H
#define ENG_TARGET_ARDUINOGEN_H

#include <string>

namespace eng {

struct Program;

namespace arduinogen {

// Generate an Arduino sketch (.ino) from a parsed EC program.
// boardId: optional board identifier (e.g. "arduino-uno"); empty = auto-detect.
std::string generateSketch(const Program& prog, const std::string& boardId = "");

} // namespace arduinogen
} // namespace eng

#endif // ENG_TARGET_ARDUINOGEN_H
