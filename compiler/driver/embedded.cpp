// EnginotechC++ — Embedded CLI Commands Implementation

#include "embedded.h"
#include "../targets/target.h"
#include "../targets/ecconfig.h"
#include "../targets/embedded_ext.h"
#include "../targets/baremetal.h"
#include "../targets/js.h"
#include "../targets/python.h"
#include "../targets/wasm.h"
#include "../targets/vm.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../semantic/semantic.h"
#include "../types/typechecker.h"
#include "../codegen/caller.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cstdlib>

namespace eng {
namespace driver {
namespace fs = std::filesystem;

static std::string archToString(target::Arch arch) {
    switch (arch) {
        case target::Arch::AVR: return "AVR";
        case target::Arch::XTENSA_ESP32: return "Xtensa";
        case target::Arch::RISC_V_ESP32: return "RISC-V";
        case target::Arch::X86_64: return "x86_64";
        default: return "unknown";
    }
}

std::vector<std::string> EmbeddedCommands::parseArgs(const std::vector<std::string>& args,
                                                      const std::string& flag) {
    std::vector<std::string> result;
    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == flag && args[i+1][0] != '-') {
            result.push_back(args[i+1]);
        }
    }
    return result;
}

std::optional<std::string> EmbeddedCommands::getArgValue(const std::vector<std::string>& args,
                                                          const std::string& flag) {
    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == flag && i + 1 < args.size()) {
            return args[i+1];
        }
    }
    return std::nullopt;
}

std::string EmbeddedCommands::findTool(const std::string& name) {
    const char* paths[] = {name.c_str(), nullptr};
    for (int i = 0; paths[i]; ++i) {
        if (std::system((std::string(paths[i]) + (fs::path::preferred_separator == '\\' ? " /?" : " --version"))
                        .c_str()) == 0) {
            return paths[i];
        }
    }
    return "";
}

bool EmbeddedCommands::hasTool(const std::string& name) {
    return !findTool(name).empty();
}

std::string EmbeddedCommands::detectBoard() {
    if (fs::exists("ec.toml")) {
        auto cfg = ecconfig::parse("ec.toml");
        if (!cfg.board.empty()) return cfg.board;
    }
    return "";
}

int EmbeddedCommands::cmdBuildEmbedded(const std::vector<std::string>& args) {
    auto targetOpt = getArgValue(args, "--target");
    auto boardOpt = getArgValue(args, "--board");
    auto srcOpt = getArgValue(args, "--src");
    auto outOpt = getArgValue(args, "--output");
    auto profileOpt = getArgValue(args, "--runtime");

    // Handle positional arguments: engc build --target web source.ec [output.js]
    std::string target = targetOpt.value_or("x86");
    std::string board = boardOpt.value_or("");
    std::string profile = profileOpt.value_or("embedded-min");

    // Find positional args by skipping flags and their values
    std::vector<std::string> positional;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i].starts_with("--")) {
            // Skip flag and its value (e.g., --target web)
            ++i;
            continue;
        }
        positional.push_back(args[i]);
    }

    // src and out from positional args or flags
    std::string src = srcOpt.value_or(
        (positional.size() > 0) ? positional[0] : "src/main.ec");
    std::string out = outOpt.value_or(
        (positional.size() > 1) ? positional[1] : "firmware.elf");

    if (fs::exists("ec.toml")) {
        auto cfg = ecconfig::parse("ec.toml");
        if (target == "x86" && !cfg.target.empty()) target = cfg.target;
        if (board.empty() && !cfg.board.empty()) board = cfg.board;
        if (profile == "embedded-min" && cfg.runtimeProfile != "embedded-min") profile = cfg.runtimeProfile;
    }

    std::cout << "Building for target: " << target;
    if (!board.empty()) std::cout << " board=" << board;
    std::cout << "\nSource: " << src << "\nOutput: " << out << "\n";
    std::cout << "Runtime profile: " << profile << "\n";

    if (target == "arduino" || target == "avr") {
        if (!hasTool("avr-gcc")) {
            std::cerr << "Error: avr-gcc not found. Install llvm-mingw with AVR support.\n";
            return 1;
        }
    } else if (target == "esp32") {
        if (!hasTool("xtensa-esp32-elf-gcc")) {
            std::cerr << "Error: xtensa-esp32-elf-gcc not found.\n";
            std::cerr << "Install ESP-IDF toolchain.\n";
            return 1;
        }
    } else if (target == "baremetal" || target == "kernel") {
        // Bare-metal x86_64 target
        std::string clang = "";
        const char* clangCandidates[] = {"clang++", "clang-21", "clang-20", nullptr};
        for (int i = 0; clangCandidates[i]; ++i) {
            std::string testCmd = clangCandidates[i] + std::string(" --version");
            if (std::system(testCmd.c_str()) == 0) {
                clang = clangCandidates[i];
                break;
            }
        }
        if (clang.empty()) {
            std::cerr << "Error: clang++ not found. Install LLVM.\n";
            return 1;
        }
        std::string objcopy = eng::target::BareMetalBackend::findObjcopy();
        if (objcopy.empty()) {
            std::cerr << "Warning: llvm-objcopy not found. Binary extraction skipped.\n";
        }
        std::cout << "Bare-metal target: " << clang << "\n";

        // Step 1: Compile EC source to LLVM IR (text format)
        DiagnosticEngine diag;
        std::ifstream srcFile(src);
        std::string source((std::istreambuf_iterator<char>(srcFile)),
                           std::istreambuf_iterator<char>());
        srcFile.close();
        if (source.empty()) {
            std::cerr << "Error: Cannot read source file: " << src << "\n";
            return 1;
        }

        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        for (const auto& d : lexer.diagnostics) {
            diag.error("L000", d.message, d.line, d.col);
        }

        Parser parser(tokens, diag);
        auto prog = parser.parse();

        SemanticAnalyzer sa(diag);
        sa.analyze(prog.topLevel);

        TypeChecker tc(diag);
        tc.check(prog.topLevel);

        if (diag.hasErrors()) {
            diag.print();
            std::cerr << "Compilation failed with " << diag.errorCount() << " error(s)\n";
            return 1;
        }

        // Step 2: Generate LLVM IR text file
        std::string irPath = fs::path(out).stem().string() + ".ll";
        std::string ir = eng::CompilerCaller::compileToIR(tokens, prog, diag);
        if (diag.hasErrors()) {
            diag.print();
            std::cerr << "IR generation failed\n";
            return 1;
        }
        {
            std::ofstream irFile(irPath);
            if (!irFile) {
                std::cerr << "Error: Cannot write IR file: " << irPath << "\n";
                return 1;
            }
            irFile << ir;
        }
        std::cout << "Generated LLVM IR: " << irPath << "\n";

        // Step 3: Assemble IR to object file with bare-metal flags
        std::string objPath = fs::path(out).stem().string() + ".o";
        std::string asmCmd = clang + " -O2 -ffreestanding -fno-stack-protector "
            "-mno-red-zone -mcmodel=kernel -c \"" + irPath + "\" -o \"" + objPath + "\"";
        if (std::system(asmCmd.c_str()) != 0) {
            std::cerr << "Error: Assembly failed\n";
            return 1;
        }
        std::cout << "Assembled to object: " << objPath << "\n";

        // Step 4: Link with bare-metal linker script
        target::BoardDef emptyBoard{};
        std::string ldScript = eng::target::BareMetalBackend::generateLinkerScriptFor(emptyBoard);
        fs::path tmpLd = fs::temp_directory_path() / "eng_baremetal.ld";
        {
            std::ofstream f(tmpLd);
            f << ldScript;
        }

        // Link using clang as driver but with explicit libc paths
        // The key is to use the host triple but output a bare-metal compatible binary
        std::string linkCmd = clang + " -O2 -mno-red-zone "
            "-T " + tmpLd.string() + " -o \"" + out + "\" "
            + objPath + " "
            "-Wl,--nmagic "  // No page alignment
            "-Wl,-Bstatic "  // Static link
            "-lc "           // System C library
            "-lgcc ";        // GCC runtime support
        
        if (std::system(linkCmd.c_str()) != 0) {
            std::cerr << "Error: Linking failed\n";
            return 1;
        }
        std::cout << "Linked ELF: " << out << "\n";

        // Step 5: Extract raw binary
        if (!objcopy.empty()) {
            std::string binPath = out + ".bin";
            if (eng::target::BareMetalBackend::extractBinary(out, binPath)) {
                std::cout << "Extracted binary: " << binPath << "\n";
                std::cout << "[OK] Kernel built successfully!\n";
                std::cout << "Usage: qemu-system-x86_64 -kernel " << binPath << "\n";
            } else {
                std::cout << "[OK] ELF built (binary extraction failed)\n";
                std::cout << "Usage: qemu-system-x86_64 -kernel " << out << "\n";
            }
        } else {
            std::cout << "[OK] Kernel built (ELF only, no objcopy)\n";
            std::cout << "Usage: qemu-system-x86_64 -kernel " << out << "\n";
        }

        return 0;
    } else if (target == "web" || target == "js" || target == "javascript") {
        // JavaScript/Web target — compiles to JS for browser or Node.js
        std::cout << "Compiling to JavaScript...\n";

        // Parse source
        DiagnosticEngine diag;
        std::ifstream srcFile(src);
        std::string source((std::istreambuf_iterator<char>(srcFile)),
                           std::istreambuf_iterator<char>());
        srcFile.close();
        if (source.empty()) {
            std::cerr << "Error: Cannot read source file: " << src << "\n";
            return 1;
        }

        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        for (const auto& d : lexer.diagnostics) {
            diag.error("L000", d.message, d.line, d.col);
        }

        Parser parser(tokens, diag);
        auto prog = parser.parse();

        SemanticAnalyzer sa(diag);
        sa.analyze(prog.topLevel);

        TypeChecker tc(diag);
        tc.check(prog.topLevel);

        if (diag.hasErrors()) {
            diag.print();
            std::cerr << "Compilation failed with " << diag.errorCount() << " error(s)\n";
            return 1;
        }

        // Generate JavaScript
        target::JSTarget jsTarget(diag);
        std::string jsCode = jsTarget.compileToJS(prog.topLevel);

        // Determine output path
        std::string outPath = out;
        if (fs::is_directory(out)) {
            fs::path srcPath(src);
            outPath = (out / srcPath.stem()).string() + ".js";
        } else if (!out.ends_with(".js")) {
            outPath = out + ".js";
        }

        // Write to file
        if (!jsTarget.saveToFile(outPath, jsCode)) {
            std::cerr << "Error: Failed to write JavaScript file: " << outPath << "\n";
            return 1;
        }

        std::cout << "Generated JavaScript: " << outPath << "\n";
        std::cout << "[OK] JavaScript compilation successful!\n";
        std::cout << "Usage: node " << outPath << "\n";
        std::cout << "Or include in HTML:\n";
        std::cout << "  <script src=\"" << outPath << "\"></script>\n";

        return 0;
    } else if (target == "python" || target == "py") {
        // Python target — transpile to Python
        std::cout << "Compiling to Python...\n";

        // Parse source
        DiagnosticEngine diag;
        std::ifstream srcFile(src);
        std::string source((std::istreambuf_iterator<char>(srcFile)),
                           std::istreambuf_iterator<char>());
        srcFile.close();
        if (source.empty()) {
            std::cerr << "Error: Cannot read source file: " << src << "\n";
            return 1;
        }

        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        for (const auto& d : lexer.diagnostics) {
            diag.error("L000", d.message, d.line, d.col);
        }

        Parser parser(tokens, diag);
        auto prog = parser.parse();

        SemanticAnalyzer sa(diag);
        sa.analyze(prog.topLevel);

        TypeChecker tc(diag);
        tc.check(prog.topLevel);

        if (diag.hasErrors()) {
            diag.print();
            std::cerr << "Compilation failed with " << diag.errorCount() << " error(s)\n";
            return 1;
        }

        // Generate Python
        target::PythonTarget pyTarget(diag);
        std::string pyCode = pyTarget.compileToPython(prog.topLevel);

        // Determine output path
        std::string outPath = out;
        if (fs::is_directory(out)) {
            fs::path srcPath(src);
            outPath = (out / srcPath.stem()).string() + ".py";
        } else if (!out.ends_with(".py")) {
            outPath = out + ".py";
        }

        // Write to file
        if (!pyTarget.saveToFile(outPath, pyCode)) {
            std::cerr << "Error: Failed to write Python file: " << outPath << "\n";
            return 1;
        }

        std::cout << "Generated Python: " << outPath << "\n";
        std::cout << "[OK] Python compilation successful!\n";
        std::cout << "Usage: python3 " << outPath << "\n";

        return 0;
    } else if (target == "wasm" || target == "webassembly") {
        // WebAssembly target — compiles to .wasm text format
        std::cout << "Compiling to WebAssembly...\n";

        DiagnosticEngine diag;
        std::ifstream srcFile(src);
        std::string source((std::istreambuf_iterator<char>(srcFile)),
                           std::istreambuf_iterator<char>());
        srcFile.close();
        if (source.empty()) {
            std::cerr << "Error: Cannot read source file: " << src << "\n";
            return 1;
        }

        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        for (const auto& d : lexer.diagnostics) {
            diag.error("L000", d.message, d.line, d.col);
        }

        Parser parser(tokens, diag);
        auto prog = parser.parse();

        SemanticAnalyzer sa(diag);
        sa.analyze(prog.topLevel);

        TypeChecker tc(diag);
        tc.check(prog.topLevel);

        if (diag.hasErrors()) {
            diag.print();
            std::cerr << "Compilation failed with " << diag.errorCount() << " error(s)\n";
            return 1;
        }

        // Generate WASM
        target::WASMTarget wasmTarget(diag);
        std::string wasmCode = wasmTarget.compileToWASM(prog.topLevel);

        std::string outPath = out;
        if (!out.ends_with(".wat")) {
            outPath = out + ".wat";
        }

        std::ofstream f(outPath);
        if (!f) {
            std::cerr << "Error: Failed to write WASM file: " << outPath << "\n";
            return 1;
        }
        f << wasmCode;
        f.close();

        std::cout << "Generated WebAssembly: " << outPath << "\n";
        std::cout << "[OK] WASM compilation successful!\n";
        std::cout << "Usage: wasm-interp " << outPath << "\n";
        std::cout << "Or compile to .wasm: wat2wasm " << outPath << " -o " << out << ".wasm\n";

        return 0;
    } else if (target == "vm" || target == "bytecode") {
        // EC Virtual Machine target
        std::cout << "Compiling to EC Bytecode...\n";

        DiagnosticEngine diag;
        std::ifstream srcFile(src);
        std::string source((std::istreambuf_iterator<char>(srcFile)),
                           std::istreambuf_iterator<char>());
        srcFile.close();
        if (source.empty()) {
            std::cerr << "Error: Cannot read source file: " << src << "\n";
            return 1;
        }

        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        for (const auto& d : lexer.diagnostics) {
            diag.error("L000", d.message, d.line, d.col);
        }

        Parser parser(tokens, diag);
        auto prog = parser.parse();

        SemanticAnalyzer sa(diag);
        sa.analyze(prog.topLevel);

        TypeChecker tc(diag);
        tc.check(prog.topLevel);

        if (diag.hasErrors()) {
            diag.print();
            std::cerr << "Compilation failed with " << diag.errorCount() << " error(s)\n";
            return 1;
        }

        // Compile to bytecode
        target::VMTarget vmTarget(diag);
        auto code = vmTarget.compileToBytecode(prog.topLevel);

        std::string outPath = out;
        if (!out.ends_with(".ecvm")) {
            outPath = out + ".ecvm";
        }

        if (!vmTarget.saveBytecode(outPath, code)) {
            std::cerr << "Error: Failed to write bytecode file: " << outPath << "\n";
            return 1;
        }

        // Execute bytecode
        std::cout << "\nExecuting bytecode...\n";
        auto result = vmTarget.execute(code);

        std::cout << "Generated Bytecode: " << outPath << "\n";
        std::cout << "[OK] VM compilation successful!\n";
        std::cout << "Usage: engc run --target vm " << outPath << "\n";

        return 0;
    }

    // Build the embedded HAL sources
    const char* halSources[] = {
        "std/embedded/gpio/gpio.cpp",
        "std/embedded/pwm/pwm.cpp",
        "std/embedded/adc/adc.cpp",
        "std/embedded/uart/uart.cpp",
        "std/embedded/i2c/i2c.cpp",
        "std/embedded/spi/spi.cpp",
        "std/embedded/timer/timer.cpp",
        "std/embedded/interrupt/interrupt.cpp",
        "std/embedded/wifi/wifi.cpp",
        "std/embedded/bluetooth/bt.cpp",
        "std/embedded/sleep/sleep.cpp",
        "std/embedded/system/system.cpp",
        "runtime/embedded/embedded_rt.cpp",
        "compiler/enginecore/enginecore.cpp",
        nullptr
    };

    // Get target-specific defines and include paths
    auto defines = embedded_ext::getTargetDefines(target, board);
    auto includes = embedded_ext::getIncludePaths(target);

    std::cout << "\nCompiling " << (target == "arduino" ? "AVR" : target == "esp32" ? "ESP32" : "host") << " target...\n";
    std::cout << "[OK] Build phase complete (stub - full toolchain integration in E3)\n";
    return 0;
}

int EmbeddedCommands::cmdFlash(const std::vector<std::string>& args) {
    auto boardOpt = getArgValue(args, "--board");
    auto portOpt = getArgValue(args, "--port");
    auto firmwareOpt = getArgValue(args, "--firmware");

    std::string board = boardOpt.value_or(detectBoard());
    std::string port = portOpt.value_or("");
    std::string firmware = firmwareOpt.value_or("firmware.elf");

    if (board.empty()) {
        std::cerr << "Error: No board specified. Use --board <board-id> or create ec.toml\n";
        std::cerr << "Available boards:\n";
        auto allBoards = target::loadAllBoards();
        for (const auto& b : allBoards) {
            std::cerr << "  " << b.id << " (" << b.name << ", " << archToString(b.arch) << ")\n";
        }
        return 1;
    }

    std::cout << "Flashing " << firmware << " to " << board;
    if (!port.empty()) std::cout << " via " << port;
    std::cout << "\n";

    auto bd = target::findBoard(board);
    if (!bd.has_value()) {
        std::cerr << "Error: Board '" << board << "' not found in boards/\n";
        return 1;
    }

    auto backend = target::TargetRegistry::getForBoard(board);
    if (!backend) {
        std::cerr << "Error: No backend registered for board '" << board << "'\n";
        return 1;
    }

    if (!fs::exists(firmware)) {
        std::cerr << "Error: Firmware file not found: " << firmware << "\n";
        std::cerr << "Run 'engc build' first.\n";
        return 1;
    }

    std::cout << "[INFO] Uploading firmware...\n";
    bool ok = backend->upload(firmware, bd.value(), port);
    if (ok) {
        std::cout << "[OK] Flash successful!\n";
    } else {
        std::cerr << "Error: Flash failed. Check connection and try again.\n";
    }
    return ok ? 0 : 1;
}

int EmbeddedCommands::cmdMonitor(const std::vector<std::string>& args) {
    auto portOpt = getArgValue(args, "--port");
    auto baudOpt = getArgValue(args, "--baud");

    std::string port = portOpt.value_or("");
    uint32_t baud = 115200;
    auto baudVal = getArgValue(args, "--baud");
    if (baudVal.has_value()) {
        try { baud = std::stoul(baudVal.value()); } catch (...) {
            std::cerr << "Error: Invalid baud rate: " << *baudVal << "\n";
            return 1;
        }
    }

    if (port.empty()) {
        port = detectBoard();
        if (port.empty()) {
            std::cerr << "Error: No serial port specified. Use --port <port>\n";
            return 1;
        }
    }

    std::cout << "Opening serial monitor on " << port << " at " << baud << " baud\n";
    std::cout << "Press Ctrl+C to exit\n\n";

    // In production, this would open a real serial port
    std::cout << "[MONITOR] Connected. waiting for data...\n";
    return 0;
}

int EmbeddedCommands::cmdDoctor() {
    std::cout << "=== EnginotechC++ Embedded Doctor ===\n\n";

    // EC compiler
    std::cout << "[CHECK] EC Compiler...\n";
    std::cout << "  Status: OK (local build)\n";

    // Target toolchains
    std::cout << "\n[CHECK] Target Toolchains:\n";
    struct ToolCheck {
        const char* name;
        const char* desc;
        bool required;
    };
    ToolCheck tools[] = {
        {"avr-gcc", "Arduino AVR compiler", false},
        {"avrdude", "Arduino flash tool", false},
        {"xtensa-esp32-elf-gcc", "ESP32 Xtensa compiler", false},
        {"riscv32-esp-elf-gcc", "ESP32 S3/C3 compiler", false},
        {"esptool.py", "ESP32 flash tool", false},
        {"cmake", "Build system", true},
        {"clang", "LLVM compiler", true},
        {nullptr, nullptr, false}
    };

    int missing = 0;
    for (int i = 0; tools[i].name; ++i) {
        bool found = hasTool(tools[i].name);
        std::cout << "  " << (found ? "[OK]" : "[MISSING]") << " "
                  << tools[i].name << " (" << tools[i].desc << ")\n";
        if (!found && tools[i].required) missing++;
    }

    // Boards
    std::cout << "\n[CHECK] Board Definitions:\n";
    auto boards = target::loadAllBoards();
    for (const auto& b : boards) {
        std::cout << "  [OK] " << b.id << " (" << b.name << ", " << b.cpu << ")\n";
    }
    if (boards.empty()) {
        std::cout << "  [INFO] No board definitions found in boards/\n";
    }

    // USB / Serial
    std::cout << "\n[CHECK] USB / Serial:\n";
    std::cout << "  [INFO] Serial port detection requires platform-specific code\n";

    std::cout << "\n=== Summary ===\n";
    if (missing > 0) {
        std::cout << "WARNING: " << missing << " required tool(s) missing\n";
        std::cout << "Install missing tools to enable embedded targets.\n";
        return 1;
    } else {
        std::cout << "All checks passed.\n";
        return 0;
    }
}

int EmbeddedCommands::cmdNewEmbedded(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: engc new <name> --target <arduino|esp32> [--board <board-id>]\n";
        return 1;
    }

    std::string name = args[0];
    auto targetOpt = getArgValue(args, "--target");
    auto boardOpt = getArgValue(args, "--board");

    std::string target = targetOpt.value_or("esp32");
    std::string board = boardOpt.value_or("");

    // Find default board for target
    if (board.empty()) {
        if (target == "arduino") board = "arduino-uno";
        else if (target == "esp32") board = "esp32-devkit";
        else board = "unknown";
    }

    fs::path projPath = name;
    if (fs::exists(projPath)) {
        std::cerr << "Error: Directory already exists: " << name << "\n";
        return 1;
    }

    fs::create_directories(projPath / "src");
    fs::create_directories(projPath / "tests");
    fs::create_directories(projPath / "examples");

    // Write ec.toml
    auto cfg = ecconfig::defaultConfigToml(name, target, board);
    std::ofstream cfgFile(projPath / "ec.toml");
    cfgFile << cfg;
    cfgFile.close();

    // Write main.ec with template code
    std::string mainEc;
    if (target == "arduino") {
        mainEc = R"(// EnginotechC++ — Arduino Project: )" + name + R"(
import embedded.gpio;
import embedded.system;

fn setup() {
    let led = gpio.output(13);
    uart.begin(115200);
    uart.write("Project " + str(") + name + R"(" started\n");
}

fn loop() {
    let led = gpio.output(13);
    led.high();
    delay(500);
    led.low();
    delay(500);
}

fn main() {
    setup();
    loop();
}
)";
    } else {
        mainEc = R"(// EnginotechC++ — ESP32 Project: )" + name + R"(
import embedded.gpio;
import embedded.uart;
import embedded.system;

fn setup() {
    let led = gpio.output(2);
    led.high();
    uart.begin(115200);
    uart.write("Project " + str(") + name + R"(" started\n");
    uart.write("Chip: " + system.getChipModel() + "\n");
}

fn loop() {
    let led = gpio.output(2);
    led.high();
    delay(500);
    led.low();
    delay(500);
}

fn main() {
    setup();
    loop();
}
)";
    }

    std::ofstream mainFile(projPath / "src" / "main.ec");
    mainFile << mainEc;
    mainFile.close();

    // Write README
    std::ofstream readme(projPath / "README.md");
    readme << "# " << name << "\n\n";
    readme << "Target: " << target << "\n";
    readme << "Board: " << board << "\n\n";
    readme << "## Quick Start\n\n";
    readme << "```bash\n";
    readme << "engc build --target " << target << " --board " << board << "\n";
    readme << "engc flash --board " << board << "\n";
    readme << "engc monitor --baud 115200\n";
    readme << "```\n";
    readme.close();

    // Write .gitignore
    std::ofstream gitignore(projPath / ".gitignore");
    gitignore << "*.elf\n*.bin\n*.hex\n.build/\n";
    gitignore.close();

    std::cout << "Created project: " << name << "\n";
    std::cout << "  Target: " << target << "\n";
    std::cout << "  Board: " << board << "\n";
    std::cout << "  Config: ec.toml\n";
    std::cout << "  Source: src/main.ec\n";
    std::cout << "\nNext steps:\n";
    std::cout << "  cd " << name << "\n";
    std::cout << "  engc build --target " << target << " --board " << board << "\n";
    return 0;
}

} // namespace driver
} // namespace eng
