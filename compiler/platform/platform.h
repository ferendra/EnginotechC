// EnginotechC++ — Cross-Platform Abstraction Layer
#ifndef ENG_PLATFORM_H
#define ENG_PLATFORM_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace eng {
namespace platform {

enum class OSType {
    LINUX, FREEBSD, OPENBSD, NETBSD, MACOS, WINDOWS, UNKNOWN
};

OSType getOSType();
std::string getOSName();
std::string getOSVersion();
bool isPOSIX();
bool isWindows();

enum class ArchType {
    X86_64, AARCH64, ARMV7, RISCV64, XTENSA, UNKNOWN
};

ArchType getArchType();
std::string getArchName();
int getCPUCount();
std::string getHostname();

struct MemoryInfo {
    uint64_t total_bytes;
    uint64_t free_bytes;
    uint64_t available_bytes;
};

MemoryInfo getMemoryInfo();

namespace fs {
    std::string readText(const std::string& path);
    bool writeText(const std::string& path, const std::string& content);
    bool exists(const std::string& path);
    bool isDirectory(const std::string& path);
    bool isFile(const std::string& path);
    bool createDirectory(const std::string& path);
    bool remove(const std::string& path);
    void copyFile(const std::string& src, const std::string& dst);
    std::vector<std::string> listDirectory(const std::string& path);
}

namespace path {
    std::string join(const std::vector<std::string>& parts);
    std::string join(const std::string& a, const std::string& b);
    std::string basename(const std::string& path);
    std::string dirname(const std::string& path);
    std::string extension(const std::string& path);
    std::string normalize(const std::string& path);
    bool isAbsolute(const std::string& path);
    std::string getCurrentDir();
    std::string getUserHome();
}

namespace env {
    std::optional<std::string> get(const std::string& name);
    void set(const std::string& name, const std::string& value);
    bool exists(const std::string& name);
}

struct ProcessResult {
    int exit_code;
    std::string stdout_data;
    std::string stderr_data;
    bool success;
};

namespace process {
    ProcessResult run(const std::string& command,
                      const std::vector<std::string>& args = {},
                      const std::string& cwd = "");
    bool isAvailable(const std::string& tool);
    std::string findInPath(const std::string& name);
}

namespace terminal {
    bool supportsColor();
    int getWidth();
    void clear();
    std::string colorize(const std::string& text, int color_code);
}

namespace thread {
    using Task = std::function<void()>;
    void spawn(Task task);
    void sleepMilliseconds(uint32_t ms);
    void sleepMicroseconds(uint32_t us);
}

namespace time {
    uint64_t nowMillis();
    uint64_t nowMicros();

    struct DateTime {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
    };

    DateTime nowDateTime();
}

namespace net {
#ifdef _WIN32
    using SocketHandle = uintptr_t;
#else
    using SocketHandle = int;
#endif

    struct TCPSocket {
        SocketHandle fd;
        bool valid;
    };

    TCPSocket tcpOpen(const std::string& host, uint16_t port);
    void tcpClose(TCPSocket& sock);
    int tcpSend(TCPSocket& sock, const uint8_t* data, int len);
    int tcpReceive(TCPSocket& sock, uint8_t* buf, int maxlen);
    std::vector<std::string> resolveDNS(const std::string& hostname);
}

namespace capabilities {
    bool hasEpoll();
    bool hasKqueue();
}

struct SystemInfo {
    std::string os;
    std::string arch;
    std::string os_version;
    std::string hostname;
    int cpu_count;
    MemoryInfo memory;
};

SystemInfo getSystemInfo();

} // namespace platform
} // namespace eng

#endif // ENG_PLATFORM_H
