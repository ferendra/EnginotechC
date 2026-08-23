// EnginotechC++ — Bare-metal x86_64 Backend Implementation

#include "baremetal.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace eng {
namespace target {
namespace fs = std::filesystem;

// ── Startup code for bare-metal ───────────────────────────────────
// Minimal entry point: sets up stack, clears BSS, calls main(), halts CPU.
static const char* BAREMETAL_STARTUP_ASM = R"(
    .intel_syntax noprefix
    .globl _start
    .section .text
    .balign 16

_start:
    # Disable interrupts
    cli

    # Set up a minimal stack (top of 4GB address space)
    mov rsp, 0x7FFFFFFF

    # Clear BSS segment (zero-initialized globals)
    xor rax, rax
    mov rdi, __bss_start
    mov rsi, __bss_end
    sub rsi, rdi
    cld
    rep stosq

    # Call C main (returns exit code in eax)
    xor rdi, rdi        # argc = 1 (we pass empty argv)
    lea rsi, [rel argv_dummy]
    call main

    # HALT: if main returns, halt the CPU
halt_loop:
    hlt
    jmp halt_loop

    .section .data
    .balign 8
argv_dummy:
    .quad 0
    .size argv_dummy, 8
)";

// ── Linker script template ────────────────────────────────────────
// Places .text at 0x100000 (classic kernel load address),
// .rodata after text, .data and .bss after that.
static std::string generateLinkerScriptInternal(const BoardDef& board) {
    // Default: 2MB binary size limit
    size_t max_size = board.flashBytes > 0 ? board.flashBytes : 2 * 1024 * 1024;

    std::ostringstream ld;
    ld << "OUTPUT_FORMAT(\"elf64-x86-64\")\n";
    ld << "OUTPUT_ARCH(i386:x86-64)\n";
    ld << "ENTRY(_start)\n\n";
    ld << "SECTIONS\n{\n";
    ld << "  . = 0x" << std::hex << max_size / 2 << ";\n\n";  // Load at half flash
    ld << "  .text ALIGN(4096) : {\n";
    ld << "    *(.text .text.*)\n";
    ld << "  }\n\n";
    ld << "  .rodata ALIGN(4096) : {\n";
    ld << "    *(.rodata .rodata.*)\n";
    ld << "  }\n\n";
    ld << "  .data ALIGN(4096) : {\n";
    ld << "    __data_start = .;\n";
    ld << "    *(.data .data.*)\n";
    ld << "  }\n\n";
    ld << "  .bss ALIGN(4096) : {\n";
    ld << "    __bss_start = .;\n";
    ld << "    *(.bss .bss.*)\n";
    ld << "    *(COMMON)\n";
    ld << "    __bss_end = .;\n";
    ld << "  }\n\n";
    ld << "  /DISCARD/ : {\n";
    ld << "    *(.eh_frame)\n";
    ld << "    *(.note.*)\n";
    ld << "  }\n";
    ld << "}\n";
    return ld.str();
}

// ── Implementation ────────────────────────────────────────────────

std::vector<std::string> BareMetalBackend::compileFlags(const TargetSpec&) const {
    return {
        "-O2", "-ffreestanding", "-fno-stack-protector",
        "-fno-rtti", "-fno-exceptions",
        "-mno-red-zone", "-mcmodel=kernel",
        "-c"
    };
}

std::vector<std::string> BareMetalBackend::linkFlags(const TargetSpec& spec) const {
    std::vector<std::string> flags;

    // Generate linker script to temp file
    std::string ld_script = generateLinkerScriptInternal(spec.board);
    fs::path tmp_path = fs::temp_directory_path() / "eng_baremetal.ld";
    std::ofstream ld_file(tmp_path);
    ld_file << ld_script;
    ld_file.close();

    flags.push_back("-T");
    flags.push_back(tmp_path.string());
    flags.push_back("-nostdlib");
    flags.push_back("-Wl,--nmagic");  // No page alignment, flat binary friendly
    return flags;
}

std::string BareMetalBackend::generateStartup(const TargetSpec&) const {
    return BAREMETAL_STARTUP_ASM;
}

std::string BareMetalBackend::findObjcopy() {
    // Try common paths
    const char* candidates[] = {
        "llvm-objcopy",
        "llvm-objcopy-21",
        "objcopy",
        "x86_64-linux-gnu-objcopy",
        nullptr
    };
    for (const char** c = candidates; *c; ++c) {
        if (std::system(("which " + std::string(*c) + " > /dev/null 2>&1").c_str()) == 0) {
            return *c;
        }
    }
    return "llvm-objcopy";  // Return default even if not found
}

bool BareMetalBackend::extractBinary(const std::string& elfPath,
                                      const std::string& binPath) {
    std::string objcopy = findObjcopy();
    std::string cmd = objcopy + " --output-target=binary \"" + elfPath + "\" \"" + binPath + "\"";
    return std::system(cmd.c_str()) == 0;
}

bool BareMetalBackend::linkFirmware(const std::string& objPath,
                                     const std::string& outPath,
                                     const TargetSpec& spec) const {
    (void)spec;
    std::string objcopy = findObjcopy();
    std::string binPath = outPath + ".bin";

    // First, produce ELF
    // Generate linker script to temp file
    std::string ld_script = generateLinkerScriptInternal(spec.board);
    fs::path tmp_ld = fs::temp_directory_path() / "eng_baremetal.ld";
    {
        std::ofstream f(tmp_ld);
        f << ld_script;
    }

    // Link to ELF
    std::string link_cmd = "clang++ -target x86_64-unknown-elf "
        "-T " + tmp_ld.string() + " -o \"" + outPath + "\" "
        "-nostdlib " + objPath;
    if (std::system(link_cmd.c_str()) != 0) {
        fprintf(stderr, "ERROR: Linking failed\n");
        return false;
    }

    // Extract raw binary
    if (extractBinary(outPath, binPath)) {
        std::printf("Compiled: %s\n", binPath.c_str());
    } else {
        std::printf("Compiled: %s (ELF only, objcopy unavailable)\n", outPath.c_str());
    }
    return true;
}

bool BareMetalBackend::upload(const std::string&, const BoardDef&, const std::string&) const {
    return false;  // No upload mechanism for bare-metal x86
}

std::vector<BoardDef> BareMetalBackend::detectBoards() const {
    return {};
}

// Public wrapper for external use (embedded.cpp)
std::string eng::target::BareMetalBackend::generateLinkerScriptFor(const BoardDef& board) {
    return generateLinkerScriptInternal(board);
}

} // namespace target
} // namespace eng
