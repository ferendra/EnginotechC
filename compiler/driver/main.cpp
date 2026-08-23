// EnginotechC++ Compiler Driver — engc
// Main entry point for the CLI

#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../semantic/semantic.h"
#include "../types/typechecker.h"
#include "../codegen/caller.h"
#include "../targets/target.h"
#include "../targets/ecconfig.h"
#include "../targets/embedded_ext.h"
#include "../targets/arduinogen.h"
#include "../targets/avr.h"
#include "../targets/esp32.h"
#include "../targets/x86.h"
#include "../targets/baremetal.h"
#include "../../packages/registry/registry.h"
#include "../../packages/manager/manager.h"
#include "../../langserver/langserver.h"
#include "embedded.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

namespace eng {
namespace fs = std::filesystem;

class EngCompiler {
public:
    struct Result {
        bool success;
        std::string error;
        std::string output;
        int exitCode;
    };

    static Result run(const std::vector<std::string>& args);
    static Result cmdVersion();
    static Result cmdNew(const std::string& path);
    static Result cmdRun(const std::string& sourcePath, const std::string& outputPath);
    static Result cmdRunEmbedded(const std::string& sourcePath,
                                  const std::string& outputPath,
                                  bool arduinoMode);
    static Result cmdBuild(const std::string& sourcePath, const std::string& outputPath);
    static Result cmdTest(const std::string& testDir);
    static Result cmdFmt(const std::string& sourcePath);
    static Result cmdLint(const std::string& sourcePath);
    static Result cmdDoc(const std::string& sourcePath);
    static Result cmdHelp();
    static Result cmdLSP();
    // Embedded commands
    static Result cmdBuildEmbedded(const std::vector<std::string>& args);
    static Result cmdFlash(const std::vector<std::string>& args);
    static Result cmdMonitor(const std::vector<std::string>& args);
    static Result cmdDoctor();
    static Result cmdNewEmbedded(const std::vector<std::string>& args);
    // Package commands
    static Result cmdPackageAdd(const std::vector<std::string>& args);
    static Result cmdPackageRemove(const std::vector<std::string>& args);
    static Result cmdPackageList();
    static Result cmdPackageSearch(const std::string& query);
    static Result cmdPackageShow(const std::string& name);
    static Result cmdPackageInit();
};

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::vector<std::string> readSourceLines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    if (!file) return lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

#ifdef _WIN32
static const char* exeSuffix() { return ".exe"; }
#else
static const char* exeSuffix() { return ""; }
#endif

EngCompiler::Result EngCompiler::cmdVersion() {
#ifndef ENG_VERSION
#define ENG_VERSION "dev"
#endif
    return {true, "", "EnginotechC++ v" ENG_VERSION "\n", 0};
}

EngCompiler::Result EngCompiler::cmdNew(const std::string& path) {
    fs::path p(path);
    if (fs::exists(p)) {
        return {false, "Directory already exists: " + path, "", 1};
    }
    fs::create_directories(p / "src");
    fs::create_directories(p / "tests");
    fs::create_directories(p / "docs");

    std::ofstream cfg(p / ".engc");
    cfg << "# EnginotechC++ Project Config\nversion = 1\nname = \"" << p.filename().string() << "\"\n";

    std::ofstream readme(p / "README.md");
    readme << "# " << p.filename().string() << "\n\nAn EnginotechC++ project.\n";

    std::ofstream main_(p / "src" / "main.ec");
    main_ << "fn main() {\n    print(\"Hello from " << p.filename().string() << "!\");\n}\n";

    return {true, "", "Created project at: " + path, 0};
}

EngCompiler::Result EngCompiler::cmdRun(const std::string& sourcePath, const std::string& outputPath) {
    printf("cmdRun: %s -> %s\n", sourcePath.c_str(), outputPath.c_str()); fflush(stderr);
    return cmdRunEmbedded(sourcePath, outputPath, false);
}

// Helper: runs a .ec file, optionally targeting Arduino (sketch gen) or LLVM native
static bool detectEmbeddedPattern(const Program& prog) {
    (void)prog;
    return false; // let explicit --target flag decide
}

EngCompiler::Result EngCompiler::cmdRunEmbedded(const std::string& sourcePath,
                                                 const std::string& outputPath,
                                                 bool arduinoMode) {
    DiagnosticEngine diag;
    FILE* f = fopen("/tmp/typecheck_debug.txt", "w"); fprintf(f, "cmdRunEmbedded START\n"); fclose(f);
    std::string source = readFile(sourcePath);
    if (source.empty()) {
        return {false, "Cannot read source file: " + sourcePath, "", 1};
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

    // Front-end failed? Report everything gathered so far in one pass.
    if (diag.hasErrors()) {
        auto srcLines = readSourceLines(sourcePath);
        diag.print(srcLines);
        return {false, "Compilation failed: " + std::to_string(diag.errorCount()) +
                           " error(s) collected", "", 1};
    }

    // ── Arduino target → generate .ino sketch ──
    if (arduinoMode || detectEmbeddedPattern(prog)) {
        std::string sketch = arduinogen::generateSketch(prog, "arduino");
        std::string sketchPath = fs::path(outputPath).stem().string() + ".ino";
        std::ofstream sk(sketchPath);
        if (!sk) {
            return {false, "Cannot write sketch: " + sketchPath, "", 1};
        }
        sk << sketch;
        sk.close();
        std::cout << "Generated Arduino sketch: " << sketchPath << "\n";
        std::cout << "─── sketch content ───\n" << sketch << "\n─── end ───\n";
        return {true, "", sketchPath, 0};
    }

    // ── Native target → LLVM IR pipeline ──
    std::string irPath = fs::path(outputPath).stem().string() + ".ll";

    if (!CompilerCaller::compileToNative(tokens, prog, outputPath, diag)) {
        auto srcLines = readSourceLines(sourcePath);
        diag.print(srcLines);
        return {false, "Code generation failed", "", 1};
    }

    diag.print();
    std::string outPath = outputPath + exeSuffix();
    std::cout << "Compiled: " << outPath << "\n";

    std::string runCmd = "\"" + outPath + "\"";
    int rc = std::system(runCmd.c_str());
    if (rc != 0) {
        return {false, "Program exited with code " + std::to_string(rc), "", rc};
    }
    return {true, "", "Ran: " + sourcePath, 0};
}

EngCompiler::Result EngCompiler::cmdBuild(const std::string& sourcePath, const std::string& outputPath) {
    printf("cmdBuild: %s -> %s\n", sourcePath.c_str(), outputPath.c_str()); fflush(stderr);
    return cmdRun(sourcePath, outputPath);
}

EngCompiler::Result EngCompiler::cmdTest(const std::string& testDir) {
    namespace fs = std::filesystem;
    if (!fs::exists(testDir) || !fs::is_directory(testDir)) {
        return {false, "Test directory not found: " + testDir, "", 1};
    }
    std::vector<std::string> files;
    for (const auto& entry : fs::recursive_directory_iterator(testDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".ec") {
            files.push_back(entry.path().string());
        }
    }
    std::sort(files.begin(), files.end());
    if (files.empty()) {
        std::cout << "No .ec test files found in " << testDir << "\n";
        return {true, "", "", 0};
    }
    int passed = 0, failed = 0;
    for (const auto& f : files) {
        std::string stem = fs::path(f).stem().string();
        std::string out = (fs::temp_directory_path() / ("engtest_" + stem)).string();
        std::cout << "---- " << f << " ----\n";
        auto r = cmdRun(f, out);
        // cmdRun already prints compile/run output; a zero exit code means pass.
        if (r.success) {
            ++passed;
            std::cout << "[PASS] " << f << "\n";
        } else {
            ++failed;
            std::cout << "[FAIL] " << f;
            if (!r.error.empty()) std::cout << " - " << r.error;
            std::cout << "\n";
        }
        std::error_code ec;
        fs::remove(out + exeSuffix(), ec);
    }
    std::cout << "\n" << passed << " passed, " << failed << " failed ("
              << files.size() << " total)\n";
    return {failed == 0, failed ? std::to_string(failed) + " test(s) failed" : "", "",
            failed ? 1 : 0};
}

EngCompiler::Result EngCompiler::cmdFmt(const std::string& sourcePath) {
    std::string source = readFile(sourcePath);
    if (source.empty()) {
        return {false, "Cannot read file: " + sourcePath, "", 1};
    }
    std::cout << source;
    return {true, "", "", 0};
}

 EngCompiler::Result EngCompiler::cmdLint(const std::string& sourcePath) {
    std::string source = readFile(sourcePath);
    if (source.empty()) {
        return {false, "Cannot read file: " + sourcePath, "", 1};
    }
    DiagnosticEngine diag;
    FILE* f = fopen("/tmp/typecheck_debug.txt", "w"); fprintf(f, "cmdRunEmbedded START\n"); fclose(f);
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, diag);
    auto prog = parser.parse();
    if (diag.hasErrors()) {
        diag.print();
        return {false, "Lint found errors", "", 1};
    }

    // Type checking pass
    TypeChecker tc(diag);
    tc.check(prog.topLevel);
    if (diag.hasErrors()) {
        diag.print();
        return {false, "Lint found type errors", "", 1};
    }
    std::cout << "Lint complete. No issues found.\n";
    return {true, "", "", 0};
}

EngCompiler::Result EngCompiler::cmdDoc(const std::string& sourcePath) {
    std::cout << "Documentation generation not yet implemented.\n";
    return {true, "", "", 0};
}

EngCompiler::Result EngCompiler::cmdHelp() {
    std::cout << "EnginotechC++ Compiler v" ENG_VERSION "\n\n"
              << "Usage:\n"
              << "  engc version              Show version\n"
              << "  engc new <project>        Create new project\n"
              << "  engc new <name> --target <t> [--board <b>]  Create embedded project\n"
              << "  engc run <file.ec> [out]  Compile and run\n"
              << "  engc build <file.ec> [out] Compile to binary\n"
              << "  engc build --target <t> [--board <b>]  Build for embedded target\n"
              << "  Targets: arduino, esp32, baremetal, kernel\n"
              << "  engc flash --board <b> [--port <p>]  Flash firmware\n"
              << "  engc monitor [--baud <b>] [--port <p>]  Serial monitor\n"
              << "  engc doctor               Check environment\n"
              << "  engc test [dir]           Run tests\n"
              << "  engc fmt <file.ec>        Format source\n"
              << "  engc lint <file.ec>       Lint source\n"
              << "  engc doc <file.ec>        Generate docs\n"
              << "  engc lsp                  Start language server (LSP)\n"
              << "  engc help                 Show this help\n\n"
              << "Package Manager:\n"
              << "  engc add <pkg>            Install package from registry\n"
              << "  engc remove <pkg>         Remove installed package\n"
              << "  engc list                 List all available packages\n"
              << "  engc search <keyword>     Search packages\n"
              << "  engc show <pkg>           Show package details\n"
              << "  engc init                 Initialize package manager\n";
    return {true, "", "", 0};
}

EngCompiler::Result EngCompiler::run(const std::vector<std::string>& args) {
    printf("EngCompiler::run called with %zu args\n", args.size()); fflush(stderr);
    if (args.size() < 2) return cmdHelp();

    const std::string& cmd = args[1];

    if (cmd == "version" || cmd == "v") return cmdVersion();
    else if (cmd == "new") {
        if (args.size() < 3) return {false, "Usage: engc new <project-name>", "", 1};
        // Check for embedded flags
        auto hasTarget = std::find(args.begin(), args.end(), "--target") != args.end();
        if (hasTarget) {
            return cmdNewEmbedded(std::vector<std::string>(args.begin() + 2, args.end()));
        }
        return cmdNew(args[2]);
    } else if (cmd == "run" || cmd == "r") {
        if (args.size() < 3) return {false, "Usage: engc run <file.ec> [output]", "", 1};
        // Parse optional --target flag before the source file
        int srcIdx = 2;
        bool arduinoMode = false;
        if (args[srcIdx] == "--target" && srcIdx + 1 < (int)args.size()) {
            arduinoMode = (args[srcIdx + 1] == "arduino" || args[srcIdx + 1] == "esp32");
            srcIdx += 2; // skip past --target <board>
        }
        if (arduinoMode) {
            std::string out = (srcIdx + 1 < (int)args.size()) ? args[srcIdx + 1] : std::string("main") + ".ino";
            return cmdRunEmbedded(args[srcIdx], out, true);
        }
        // Check for embedded build flag (--target means build-for-embedded, not just lint)
        bool hasTargetFlag = std::find(args.begin(), args.end(), "--target") != args.end();
        if (hasTargetFlag) {
            return cmdBuildEmbedded(std::vector<std::string>(args.begin() + 2, args.end()));
        }
        std::string out = (srcIdx + 1 < (int)args.size()) ? args[srcIdx + 1] : std::string("main") + exeSuffix();
        return cmdRun(args[srcIdx], out);
    } else if (cmd == "build" || cmd == "b") {
        // Check for embedded flags
        auto hasTarget = std::find(args.begin(), args.end(), "--target") != args.end();
        if (hasTarget) {
            return cmdBuildEmbedded(std::vector<std::string>(args.begin() + 2, args.end()));
        }
        if (args.size() < 3) return {false, "Usage: engc build <file.ec> [output]", "", 1};
        std::string out = args.size() > 3 ? args[3] : std::string("main") + exeSuffix();
        return cmdBuild(args[2], out);
    } else if (cmd == "flash") {
        return cmdFlash(std::vector<std::string>(args.begin() + 2, args.end()));
    } else if (cmd == "monitor") {
        return cmdMonitor(std::vector<std::string>(args.begin() + 2, args.end()));
    } else if (cmd == "doctor") {
        return cmdDoctor();
    } else if (cmd == "add") {
        return cmdPackageAdd(std::vector<std::string>(args.begin() + 2, args.end()));
    } else if (cmd == "remove" || cmd == "rm") {
        return cmdPackageRemove(std::vector<std::string>(args.begin() + 2, args.end()));
    } else if (cmd == "list" || cmd == "ls") {
        return cmdPackageList();
    } else if (cmd == "search" || cmd == "find") {
        return cmdPackageSearch(args.size() > 2 ? args[2] : "");
    } else if (cmd == "show") {
        if (args.size() < 3) return {false, "Usage: engc show <package>", "", 1};
        return cmdPackageShow(args[2]);
    } else if (cmd == "init") {
        return cmdPackageInit();
    } else if (cmd == "test" || cmd == "t") {
        std::string dir = args.size() > 2 ? args[2] : ".";
        return cmdTest(dir);
    } else if (cmd == "fmt") {
        if (args.size() < 3) return {false, "Usage: engc fmt <file.ec>", "", 1};
        return cmdFmt(args[2]);
    } else if (cmd == "lint") {
        if (args.size() < 3) return {false, "Usage: engc lint <file.ec>", "", 1};
        return cmdLint(args[2]);
    } else if (cmd == "doc") {
        if (args.size() < 3) return {false, "Usage: engc doc <file.ec>", "", 1};
        return cmdDoc(args[2]);
    } else if (cmd == "help" || cmd == "h" || cmd == "--help" || cmd == "-h") {
        return cmdHelp();
    } else if (cmd == "lsp" || cmd == "server") {
        return cmdLSP();
    } else {
        return {false, "Unknown command: " + cmd, "", 1};
    }
}

// Embedded command implementations
EngCompiler::Result EngCompiler::cmdBuildEmbedded(const std::vector<std::string>& args) {
    int ret = eng::driver::EmbeddedCommands::cmdBuildEmbedded(args);
    return {ret == 0, ret != 0 ? "Build failed" : "", "", ret};
}

EngCompiler::Result EngCompiler::cmdFlash(const std::vector<std::string>& args) {
    int ret = eng::driver::EmbeddedCommands::cmdFlash(args);
    return {ret == 0, ret != 0 ? "Flash failed" : "", "", ret};
}

EngCompiler::Result EngCompiler::cmdMonitor(const std::vector<std::string>& args) {
    int ret = eng::driver::EmbeddedCommands::cmdMonitor(args);
    return {ret == 0, ret != 0 ? "Monitor failed" : "", "", ret};
}

EngCompiler::Result EngCompiler::cmdDoctor() {
    int ret = eng::driver::EmbeddedCommands::cmdDoctor();
    return {ret == 0, ret != 0 ? "Doctor check failed" : "", "", ret};
}

EngCompiler::Result EngCompiler::cmdNewEmbedded(const std::vector<std::string>& args) {
    int ret = eng::driver::EmbeddedCommands::cmdNewEmbedded(args);
    return {ret == 0, ret != 0 ? "Project creation failed" : "", "", ret};
}

// Package command implementations
EngCompiler::Result EngCompiler::cmdPackageAdd(const std::vector<std::string>& args) {
    if (args.empty()) {
        return {false, "Usage: engc add <package-name>", "", 1};
    }
    auto result = eng::pkgmgr::PackageManager::add(args[0]);
    return {result, result ? "" : "Failed to add package", "", result ? 0 : 1};
}

EngCompiler::Result EngCompiler::cmdPackageRemove(const std::vector<std::string>& args) {
    if (args.empty()) {
        return {false, "Usage: engc remove <package-name>", "", 1};
    }
    auto result = eng::pkgmgr::PackageManager::remove(args[0]);
    return {result, result ? "" : "Failed to remove package", "", result ? 0 : 1};
}

EngCompiler::Result EngCompiler::cmdPackageList() {
    eng::pkgmgr::PackageManager::printList();
    return {true, "", "", 0};
}

EngCompiler::Result EngCompiler::cmdPackageSearch(const std::string& query) {
    if (query.empty()) {
        std::cout << "Usage: engc search <keyword>\n";
        std::cout << "Examples:\n";
        std::cout << "  engc search sensor\n";
        std::cout << "  engc search display\n";
        std::cout << "  engc search wifi\n";
        return {true, "", "", 0};
    }
    auto results = eng::pkgmgr::PackageManager::search(query);
    if (results.empty()) {
        std::cout << "No packages found matching '" << query << "'\n";
    } else {
        std::cout << "Found " << results.size() << " package(s) matching '" << query << "':\n\n";
        for (const auto& p : results) {
            std::cout << "  " << p.name << " v" << p.version
                      << " [" << (p.for_arduino ? "arduino" : "")
                      << (p.for_esp32 ? " esp32" : "") << "]\n";
            std::cout << "    " << p.description << "\n";
        }
    }
    return {true, "", "", 0};
}

EngCompiler::Result EngCompiler::cmdPackageShow(const std::string& name) {
    eng::pkgmgr::PackageManager::showInfo(name);
    return {true, "", "", 0};
}

EngCompiler::Result EngCompiler::cmdPackageInit() {
    auto result = eng::pkgmgr::PackageManager::init(".");
    if (result) {
        std::cout << "Initialized package manager in current directory.\n";
        std::cout << "Use 'engc add <package>' to install packages.\n";
    }
    return {result, result ? "" : "Failed to initialize", "", result ? 0 : 1};
}

EngCompiler::Result EngCompiler::cmdLSP() {
    eng::langserver::LanguageServer server;
    server.run();
    return {true, "", "", 0};
}

} // namespace eng

int main(int argc, char* argv[]) {
    fprintf(stderr, "=== MAIN START ===\n"); fflush(stderr);
    fprintf(stderr, "argc=%d\n", argc); fflush(stderr); fflush(stderr);
    printf("main: argc=%d\n", argc); fflush(stderr);
    std::vector<std::string> args(argv, argv + argc);

    // Register target backends
    eng::target::TargetRegistry::registerBackend(
        std::shared_ptr<eng::target::X86Backend>(new eng::target::X86Backend()));
    eng::target::TargetRegistry::registerBackend(
        std::shared_ptr<eng::target::AVRBackend>(new eng::target::AVRBackend()));
    eng::target::TargetRegistry::registerBackend(
        std::shared_ptr<eng::target::ESP32Backend>(new eng::target::ESP32Backend()));
    eng::target::TargetRegistry::registerBackend(
        std::shared_ptr<eng::target::BareMetalBackend>(new eng::target::BareMetalBackend()));
    eng::target::TargetRegistry::registerBackend(
        std::shared_ptr<eng::target::BareMetalBackend>(new eng::target::BareMetalBackend()));

    auto result = eng::EngCompiler::run(args);

    if (!result.error.empty()) {
        std::cerr << "Error: " << result.error << "\n";
    }
    if (!result.output.empty()) {
        std::cout << result.output;
    }

    return result.exitCode;
}
