// EnginotechC++ — Platform Implementation (Common/POSIX)
// Works on Linux, FreeBSD, OpenBSD, NetBSD, macOS, Windows (MinGW)

// Enable POSIX extensions for MinGW/GCC
#define __USE_MINGW_ANSI_STDIO 1
#define _POSIX_C_SOURCE 200112L

#include "platform.h"
#include <sys/stat.h>
#include <sys/utsname.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <unistd.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <sys/sysctl.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <TargetConditionals.h>
#endif

namespace eng {
namespace platform {

OSType getOSType() {
#ifdef __linux__
    return OSType::LINUX;
#elif defined(__FreeBSD__)
    return OSType::FREEBSD;
#elif defined(__OpenBSD__)
    return OSType::OPENBSD;
#elif defined(__NetBSD__)
    return OSType::NETBSD;
#elif defined(__APPLE__)
    return OSType::MACOS;
#elif defined(_WIN32)
    return OSType::WINDOWS;
#else
    return OSType::UNKNOWN;
#endif
}

std::string getOSName() {
    switch (getOSType()) {
        case OSType::LINUX: return "linux";
        case OSType::FREEBSD: return "freebsd";
        case OSType::OPENBSD: return "openbsd";
        case OSType::NETBSD: return "netbsd";
        case OSType::MACOS: return "macos";
        case OSType::WINDOWS: return "windows";
        default: return "unknown";
    }
}

std::string getOSVersion() {
    struct utsname buffer;
    if (uname(&buffer) == 0) return std::string(buffer.release);
    return "unknown";
}

bool isPOSIX() { return getOSType() != OSType::WINDOWS; }
bool isWindows() { return getOSType() == OSType::WINDOWS; }

ArchType getArchType() {
#ifdef __x86_64__
    return ArchType::X86_64;
#elif defined(__aarch64__)
    return ArchType::AARCH64;
#elif defined(__arm__)
    return ArchType::ARMV7;
#elif defined(__riscv) && __riscv_xlen == 64
    return ArchType::RISCV64;
#elif defined(__xtensa__)
    return ArchType::XTENSA;
#else
    return ArchType::UNKNOWN;
#endif
}

std::string getArchName() {
    switch (getArchType()) {
        case ArchType::X86_64: return "x86_64";
        case ArchType::AARCH64: return "aarch64";
        case ArchType::ARMV7: return "armv7";
        case ArchType::RISCV64: return "riscv64";
        case ArchType::XTENSA: return "xtensa";
        default: return "unknown";
    }
}

int getCPUCount() { return std::thread::hardware_concurrency(); }

std::string getHostname() {
    char name[256] = {0};
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    size_t len = sizeof(name);
    sysctlbyname("kern.hostname", name, &len, nullptr, 0);
#else
    gethostname(name, sizeof(name));
#endif
    return std::string(name);
}

MemoryInfo getMemoryInfo() {
    MemoryInfo info{};
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        info.total_bytes = si.totalram * si.mem_unit;
        info.free_bytes = si.freeram * si.mem_unit;
        info.available_bytes = info.free_bytes;
    }
#elif defined(__APPLE__)
    size_t size = sizeof(info.total_bytes);
    sysctlbyname("hw.memsize", &info.total_bytes, &size, nullptr, 0);
    info.free_bytes = info.total_bytes / 2;
    info.available_bytes = info.free_bytes;
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    size_t len = sizeof(info.total_bytes);
    sysctl(mib, 2, &info.total_bytes, &len, nullptr, 0);
    info.free_bytes = info.total_bytes / 2;
    info.available_bytes = info.free_bytes;
#endif
    return info;
}

namespace fs {
    std::string readText(const std::string& path) {
        std::ifstream f(path);
        if (!f) return "";
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    bool writeText(const std::string& path, const std::string& content) {
        std::ofstream f(path);
        if (!f) return false;
        f << content;
        return true;
    }

    bool exists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }

    bool isDirectory(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    bool isFile(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
    }

    bool createDirectory(const std::string& path) {
        return mkdir(path.c_str(), 0755) == 0;
    }

    bool remove(const std::string& path) {
        return std::remove(path.c_str()) == 0;
    }

    void copyFile(const std::string& src, const std::string& dst) {
        std::ifstream in(src, std::ios::binary);
        std::ofstream out(dst, std::ios::binary);
        out << in.rdbuf();
    }

    std::vector<std::string> listDirectory(const std::string& path) {
        std::vector<std::string> result;
        return result;
    }
}

namespace path {
    std::string join(const std::vector<std::string>& parts) {
        std::string result;
        for (const auto& p : parts) {
            if (!result.empty() && result.back() != '/') result += '/';
            result += p;
        }
        return result;
    }

    std::string join(const std::string& a, const std::string& b) {
        return a + '/' + b;
    }

    std::string basename(const std::string& path) {
        size_t pos = path.find_last_of('/');
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }

    std::string dirname(const std::string& path) {
        size_t pos = path.find_last_of('/');
        return (pos == std::string::npos) ? "." : path.substr(0, pos);
    }

    std::string extension(const std::string& path) {
        size_t pos = path.find_last_of('.');
        return (pos == std::string::npos) ? "" : path.substr(pos);
    }

    std::string normalize(const std::string& path) {
        std::string result = path;
        while (result.size() > 1 && result.back() == '/') result.pop_back();
        return result;
    }

    bool isAbsolute(const std::string& path) {
        return !path.empty() && path[0] == '/';
    }

    std::string getCurrentDir() {
        char buf[PATH_MAX];
        if (getcwd(buf, sizeof(buf))) return std::string(buf);
        return ".";
    }

    std::string getUserHome() {
        const char* home = getenv("HOME");
        return home ? std::string(home) : ".";
    }
}

namespace env {
    std::optional<std::string> get(const std::string& name) {
        const char* val = getenv(name.c_str());
        if (val) return std::string(val);
        return std::nullopt;
    }

    void set(const std::string& name, const std::string& value) {
        setenv(name.c_str(), value.c_str(), 1);
    }

    bool exists(const std::string& name) {
        return get(name).has_value();
    }
}

namespace process {
    ProcessResult run(const std::string& command,
                      const std::vector<std::string>& args,
                      const std::string& cwd) {
        (void)cwd;
        ProcessResult result{};
        std::string cmd = command;
        for (const auto& arg : args) cmd += " \"" + arg + "\"";
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) { result.exit_code = -1; result.success = false; return result; }
        
        char buffer[4096];
        std::string stdout_str;
        while (fgets(buffer, sizeof(buffer), pipe)) stdout_str += buffer;
        
        result.exit_code = pclose(pipe);
        result.stdout_data = stdout_str;
        result.success = (result.exit_code == 0);
        return result;
    }

    bool isAvailable(const std::string& tool) {
        std::string cmd = "command -v " + tool + " > /dev/null 2>&1";
        return system(cmd.c_str()) == 0;
    }

    std::string findInPath(const std::string& name) {
        auto path_opt = env::get("PATH");
        if (!path_opt) return "";
        std::stringstream ss(*path_opt);
        std::string dir;
        while (std::getline(ss, dir, ':')) {
            std::string candidate = path::join(dir, name);
            if (fs::exists(candidate) && fs::isFile(candidate)) return candidate;
        }
        return "";
    }
}

namespace terminal {
    bool supportsColor() {
        const char* term = getenv("TERM");
        const char* color = getenv("COLORTERM");
        return (term && strstr(term, "color")) || (color && strstr(color, "true"));
    }

    int getWidth() { return 80; }
    void clear() { printf("\033[2J\033[H"); }
    std::string colorize(const std::string& text, int color_code) {
        if (!supportsColor()) return text;
        return "\033[" + std::to_string(color_code) + "m" + text + "\033[0m";
    }
}

namespace thread {
    void spawn(Task task) {
        std::thread t(task);
        t.detach();
    }

    void sleepMilliseconds(uint32_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void sleepMicroseconds(uint32_t us) {
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }
}

namespace time {
    uint64_t nowMillis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    uint64_t nowMicros() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
}

namespace capabilities {
    bool hasEpoll() {
#ifdef __linux__
        return true;
#else
        return false;
#endif
    }

    bool hasKqueue() {
#ifdef __FreeBSD__
        return true;
#endif
#ifdef __OpenBSD__
        return true;
#endif
#ifdef __NetBSD__
        return true;
#endif
#ifdef __APPLE__
        return true;
#endif
        return false;
    }
}

SystemInfo getSystemInfo() {
    SystemInfo info{};
    info.os = getOSName();
    info.arch = getArchName();
    info.os_version = getOSVersion();
    info.hostname = getHostname();
    info.cpu_count = getCPUCount();
    info.memory = getMemoryInfo();
    return info;
}

} // namespace platform
} // namespace eng
