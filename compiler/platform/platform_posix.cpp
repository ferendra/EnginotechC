// EnginotechC++ — Platform Implementation (Common/POSIX)
// Works on Linux, FreeBSD, OpenBSD, NetBSD, macOS

#include "platform.h"
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <ctime>

#ifdef __linux__
#include <sys/sysinfo.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <unistd.h>
#define ENG_OS_POSIX 1
#elif defined(__FreeBSD__)
#include <sys/sysctl.h>
#include <sys/event.h>
#include <unistd.h>
#define ENG_OS_BSD 1
#elif defined(__OpenBSD__)
#include <sys/sysctl.h>
#include <sys/event.h>
#include <unistd.h>
#define ENG_OS_BSD 1
#elif defined(__NetBSD__)
#include <sys/sysctl.h>
#include <sys/event.h>
#include <unistd.h>
#define ENG_OS_BSD 1
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <TargetConditionals.h>
#if TARGET_OS_OSX
#define ENG_OS_MACOS 1
#endif
#else
#define ENG_OS_POSIX 1
#endif

namespace eng {
namespace platform {

// ======================== OS Detection ========================
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
    if (uname(&buffer) == 0) {
        return std::string(buffer.release);
    }
    return "unknown";
}

bool isPOSIX() {
    return getOSType() != OSType::WINDOWS;
}

bool isWindows() {
    return getOSType() == OSType::WINDOWS;
}

// ======================== CPU Architecture ========================
ArchType getArchType() {
#ifdef __x86_64__
    return ArchType::X86_64;
#elif defined(__aarch64__) || defined(_M_ARM64)
    return ArchType::AARCH64;
#elif defined(__arm__) || defined(_M_ARM)
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

int getCPUCount() {
    return std::thread::hardware_concurrency();
}

std::string getHostname() {
    char name[256] = {0};
    if (gethostname(name, sizeof(name)) == 0) {
        return std::string(name);
    }
    return "unknown";
}

// ======================== Memory ========================
MemoryInfo getMemoryInfo() {
    MemoryInfo info{};
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        info.total_bytes = si.totalram * si.mem_unit;
        info.free_bytes = si.freeram * si.mem_unit;
        info.available_bytes = info.free_bytes;
    }
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    size_t size = sizeof(info.total_bytes);
    sysctlbyname("hw.memsize", &info.total_bytes, &size, nullptr, 0);
    info.free_bytes = info.total_bytes / 2;  // Approximate
    info.available_bytes = info.free_bytes;
#else
    info.total_bytes = 0;
    info.free_bytes = 0;
    info.available_bytes = 0;
#endif
    return info;
}

// ======================== Filesystem ========================
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

    bool readBinary(const std::string& path, std::vector<uint8_t>& out) {
        std::ifstream f(path, std::ios::binary);
        if (!f) return false;
        out.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        return true;
    }

    bool writeBinary(const std::string& path, const std::vector<uint8_t>& data) {
        std::ofstream f(path, std::ios::binary);
        if (!f) return false;
        f.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }

    bool exists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }

    bool isDirectory(const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) != 0) return false;
        return S_ISDIR(st.st_mode);
    }

    bool isFile(const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) != 0) return false;
        return S_ISREG(st.st_mode);
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

    void renameFile(const std::string& oldPath, const std::string& newPath) {
        std::rename(oldPath.c_str(), newPath.c_str());
    }

    std::vector<std::string> listDirectory(const std::string& path) {
        std::vector<std::string> result;
        // Use simple directory iteration (cross-platform via POSIX)
        return result;  // Placeholder - full impl uses dir.h
    }

    std::string createTempFile() {
        char tmpl[] = "/tmp/engc_XXXXXX";
        int fd = mkstemp(tmpl);
        if (fd < 0) return "";
        close(fd);
        return std::string(tmpl);
    }

    std::string createTempDir() {
        char tmpl[] = "/tmp/engc_dir_XXXXXX";
        char* result = mkdtemp(tmpl);
        return result ? std::string(result) : "";
    }
}

// ======================== Paths ========================
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

    std::string join(const std::string& a, const std::string& b, const std::string& c) {
        return a + '/' + b + '/' + c;
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

    std::string stem(const std::string& path) {
        size_t dot = path.find_last_of('.');
        size_t slash = path.find_last_of('/');
        if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) {
            return path;
        }
        return path.substr(0, dot);
    }

    std::string normalize(const std::string& path) {
        // Simple normalization: remove trailing slash
        std::string result = path;
        while (result.size() > 1 && result.back() == '/') {
            result.pop_back();
        }
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

    std::string getExecutablePath() {
        char buf[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';
            return std::string(buf);
        }
        return getCurrentDir();
    }
}

// ======================== Environment ========================
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

    std::vector<std::string> getAll() {
        // Simple implementation - get common vars
        return {"PATH", "HOME", "USER", "LANG"};
    }
}

// ======================== Process ========================
namespace process {
    ProcessResult run(const std::string& command,
                      const std::vector<std::string>& args,
                      const std::string& cwd) {
        (void)cwd;
        ProcessResult result{};
        
        // Build command string
        std::string cmd = command;
        for (const auto& arg : args) {
            cmd += " \"" + arg + "\"";
        }
        
        // Execute and capture output
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            result.exit_code = -1;
            result.success = false;
            return result;
        }
        
        char buffer[4096];
        std::string stdout_str, stderr_str;
        while (fgets(buffer, sizeof(buffer), pipe)) {
            stdout_str += buffer;
        }
        
        result.exit_code = pclose(pipe);
        result.stdout_data = stdout_str;
        result.stderr_data = stderr_str;
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
        
        // Split PATH and search each directory
        std::stringstream ss(*path_opt);
        std::string dir;
        while (std::getline(ss, dir, ':')) {
            std::string candidate = path::join(dir, name);
            if (fs::exists(candidate) && fs::isFile(candidate)) {
                return candidate;
            }
        }
        return "";
    }
}

// ======================== Terminal ========================
namespace terminal {
    bool supportsColor() {
        const char* term = getenv("TERM");
        const char* color = getenv("COLORTERM");
        return (term && std::string(term).find("color") != std::string::npos) ||
               (color && std::string(color).find("true") != std::string::npos) ||
               getenv("TERM_PROGRAM");
    }

    int getWidth() {
        // Try to get terminal width
        return 80;  // Default fallback
    }

    int getHeight() {
        return 24;  // Default fallback
    }

    void clear() {
        printf("\033[2J\033[H");
    }

    void setColor(bool enabled) {
        if (enabled && supportsColor()) {
            // Terminal already supports color
        }
    }

    std::string colorize(const std::string& text, int color_code) {
        if (!supportsColor()) return text;
        return "\033[" + std::to_string(color_code) + "m" + text + "\033[0m";
    }
}

// ======================== Threads ========================
namespace thread {
    void spawn(Task task) {
        std::thread t(task);
        t.detach();
    }

    void join(void*) {
        // Not implemented for detached threads
    }

    void sleepMilliseconds(uint32_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void sleepMicroseconds(uint32_t us) {
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }

    // Mutex implementation
    struct Mutex {
        std::mutex m;
    };

    Mutex* mutexCreate() { return new Mutex(); }
    void mutexLock(Mutex* m) { m->m.lock(); }
    void mutexUnlock(Mutex* m) { m->m.unlock(); }
    void mutexDestroy(Mutex* m) { delete m; }
}

// ======================== Time ========================
namespace time {
    uint64_t nowMillis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    uint64_t nowMicros() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    DateTime nowDateTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        struct tm tm {};
        localtime_r(&time_t, &tm);
        
        DateTime dt{};
        dt.year = tm.tm_year + 1900;
        dt.month = tm.tm_mon + 1;
        dt.day = tm.tm_mday;
        dt.hour = tm.tm_hour;
        dt.minute = tm.tm_min;
        dt.second = tm.tm_sec;
        return dt;
    }
}

// ======================== Networking ========================
namespace net {
    TCPSocket tcpOpen(const std::string& host, uint16_t port) {
        // POSIX implementation using POSIX sockets
        TCPSocket sock{};
        
        struct addrinfo hints, *res;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        char port_str[6];
        snprintf(port_str, sizeof(port_str), "%d", port);
        
        if (getaddrinfo(host.c_str(), port_str, &hints, &res) != 0) {
            sock.valid = false;
            return sock;
        }
        
        sock.fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock.fd < 0) {
            sock.valid = false;
            freeaddrinfo(res);
            return sock;
        }
        
        if (connect(sock.fd, res->ai_addr, res->ai_addrlen) < 0) {
            close(sock.fd);
            sock.valid = false;
            freeaddrinfo(res);
            return sock;
        }
        
        freeaddrinfo(res);
        sock.valid = true;
        return sock;
    }

    void tcpClose(TCPSocket& sock) {
        if (sock.valid) {
            close(sock.fd);
            sock.valid = false;
        }
    }

    int tcpSend(TCPSocket& sock, const uint8_t* data, int len) {
        if (!sock.valid) return -1;
        return send(sock.fd, data, len, 0);
    }

    int tcpReceive(TCPSocket& sock, uint8_t* buf, int maxlen) {
        if (!sock.valid) return -1;
        return recv(sock.fd, buf, maxlen, 0);
    }

    std::vector<std::string> resolveDNS(const std::string& hostname) {
        std::vector<std::string> results;
        
        struct addrinfo hints, *res;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        
        if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0) {
            return results;
        }
        
        for (auto* rp = res; rp != nullptr; rp = rp->ai_next) {
            struct sockaddr_in* s = (struct sockaddr_in*)rp->ai_addr;
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &s->sin_addr, ip_str, INET_ADDRSTRLEN);
            results.push_back(std::string(ip_str));
        }
        
        freeaddrinfo(res);
        return results;
    }
}

// ======================== Dynamic Libraries ========================
namespace dl {
    struct Library {
        void* handle;
    };

    Library* open(const std::string& path) {
        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) return nullptr;
        return new Library{handle};
    }

    void close(Library* lib) {
        if (lib) {
            dlclose(lib->handle);
            delete lib;
        }
    }

    void* getSymbol(Library* lib, const std::string& name) {
        if (!lib) return nullptr;
        return dlsym(lib->handle, name.c_str());
    }

    std::string getLastError() {
        return dlerror() ? dlerror() : "";
    }
}

// ======================== Platform Capabilities ========================
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

    bool hasIOUring() {
#ifdef __linux__
        return true;  // Requires Linux 5.1+
#endif
        return false;
    }

    bool hasInotify() {
#ifdef __linux__
        return true;
#endif
        return false;
    }

    bool hasFSEvents() {
#ifdef __APPLE__
        return true;
#endif
        return false;
    }

    bool hasReadDirChanges() {
        return false;  // Windows only
    }
}

// ======================== System ========================
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
