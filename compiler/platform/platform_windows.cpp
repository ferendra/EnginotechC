// EnginotechC++ — Platform Implementation (Windows)
// Windows-specific APIs using Win32

#include "platform.h"
#include <windows.h>
#include <shlwapi.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <io.h>
#include <fcntl.h>

#pragma comment(lib, "shlwapi.lib")

namespace eng {
namespace platform {

// ======================== OS Detection ========================
OSType getOSType() {
    return OSType::WINDOWS;
}

std::string getOSName() {
    return "windows";
}

std::string getOSVersion() {
    OSVERSIONINFOEXW vi = {sizeof(vi)};
    if (GetVersionExW((OSVERSIONINFOW*)&vi)) {
        return std::to_string(vi.dwMajorVersion) + "." + 
               std::to_string(vi.dwMinorVersion) + "." +
               std::to_string(vi.dwBuildNumber);
    }
    return "unknown";
}

bool isPOSIX() { return false; }
bool isWindows() { return true; }

// ======================== CPU Architecture ========================
ArchType getArchType() {
#ifdef _M_X64
    return ArchType::X86_64;
#elif defined(_M_ARM64)
    return ArchType::AARCH64;
#elif defined(_M_IX86)
    return ArchType::X86_64;  // Treat x86 as x86_64 compatible
#else
    return ArchType::UNKNOWN;
#endif
}

std::string getArchName() {
    switch (getArchType()) {
        case ArchType::X86_64: return "x86_64";
        case ArchType::AARCH64: return "aarch64";
        default: return "unknown";
    }
}

int getCPUCount() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
}

std::string getHostname() {
    char name[256] = {0};
    DWORD size = sizeof(name);
    GetComputerNameA(name, &size);
    return std::string(name);
}

// ======================== Memory ========================
MemoryInfo getMemoryInfo() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);
    
    MemoryInfo info{};
    info.total_bytes = memInfo.ullTotalPhys;
    info.free_bytes = memInfo.ullAvailPhys;
    info.available_bytes = memInfo.ullAvailPhys;
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
        return PathFileExistsA(path.c_str()) != FALSE;
    }

    bool isDirectory(const std::string& path) {
        DWORD attrs = GetFileAttributesA(path.c_str());
        return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool isFile(const std::string& path) {
        DWORD attrs = GetFileAttributesA(path.c_str());
        return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool createDirectory(const std::string& path) {
        return CreateDirectoryA(path.c_str(), nullptr) != FALSE;
    }

    bool remove(const std::string& path) {
        return DeleteFileA(path.c_str()) != FALSE;
    }

    void copyFile(const std::string& src, const std::string& dst) {
        CopyFileA(src.c_str(), dst.c_str(), FALSE);
    }

    void renameFile(const std::string& oldPath, const std::string& newPath) {
        MoveFileA(oldPath.c_str(), newPath.c_str());
    }

    std::vector<std::string> listDirectory(const std::string& path) {
        std::vector<std::string> result;
        WIN32_FIND_DATAA fd;
        std::string searchPath = path + "\\*";
        
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) return result;
        
        do {
            std::string name = fd.cFileName;
            if (name != "." && name != "..") {
                result.push_back(path + "\\" + name);
            }
        } while (FindNextFileA(hFind, &fd));
        
        FindClose(hFind);
        return result;
    }

    std::string createTempFile() {
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        char tempFile[MAX_PATH];
        GetTempFileNameA(tempPath, "eng", 0, tempFile);
        return std::string(tempFile);
    }

    std::string createTempDir() {
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        char dirName[MAX_PATH];
        UINT result = GetTempFileNameA(tempPath, "engdir", 0, dirName);
        if (result) {
            RemoveFileA(dirName);
            CreateDirectoryA(dirName, nullptr);
            return std::string(dirName);
        }
        return "";
    }
}

// ======================== Paths ========================
namespace path {
    std::string join(const std::vector<std::string>& parts) {
        std::string result;
        for (const auto& p : parts) {
            if (!result.empty() && result.back() != '\\' && result.back() != '/') {
                result += '\\';
            } else if (result.empty() || result.back() == '\\' || result.back() == '/') {
                // Skip duplicate separators
            }
            result += p;
        }
        return result;
    }

    std::string join(const std::string& a, const std::string& b) {
        return a + "\\" + b;
    }

    std::string join(const std::string& a, const std::string& b, const std::string& c) {
        return a + "\\" + b + "\\" + c;
    }

    std::string basename(const std::string& path) {
        size_t pos = path.find_last_of("\\/");
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }

    std::string dirname(const std::string& path) {
        size_t pos = path.find_last_of("\\/");
        return (pos == std::string::npos) ? "." : path.substr(0, pos);
    }

    std::string extension(const std::string& path) {
        size_t pos = path.find_last_of('.');
        return (pos == std::string::npos) ? "" : path.substr(pos);
    }

    std::string stem(const std::string& path) {
        std::string base = basename(path);
        size_t dot = base.find_last_of('.');
        return (dot == std::string::npos) ? base : base.substr(0, dot);
    }

    std::string normalize(const std::string& path) {
        std::string result = path;
        // Replace backslashes with forward slashes
        std::replace(result.begin(), result.end(), '\\', '/');
        // Remove trailing slash
        while (result.size() > 1 && result.back() == '/') {
            result.pop_back();
        }
        return result;
    }

    bool isAbsolute(const std::string& path) {
        // Check for drive letter (C:\) or UNC (\\)
        return (path.size() >= 2 && path[1] == ':') || 
               (path.size() >= 2 && path[0] == '\\' && path[1] == '\\');
    }

    std::string getCurrentDir() {
        char buf[_MAX_PATH];
        _getcwd(buf, _MAX_PATH);
        return std::string(buf);
    }

    std::string getUserHome() {
        const char* home = getenv("USERPROFILE");
        return home ? std::string(home) : getCurrentDir();
    }

    std::string getExecutablePath() {
        char buf[_MAX_PATH];
        GetModuleFileNameA(nullptr, buf, _MAX_PATH);
        return std::string(buf);
    }
}

// ======================== Environment ========================
namespace env {
    std::optional<std::string> get(const std::string& name) {
        DWORD size = GetEnvironmentVariableA(name.c_str(), nullptr, 0);
        if (size == 0) return std::nullopt;
        
        std::string value(size - 1, '\0');
        GetEnvironmentVariableA(name.c_str(), &value[0], size);
        return value;
    }

    void set(const std::string& name, const std::string& value) {
        SetEnvironmentVariableA(name.c_str(), value.c_str());
    }

    bool exists(const std::string& name) {
        return get(name).has_value();
    }

    std::vector<std::string> getAll() {
        // Get environment block
        char* envBlock = GetEnvironmentStringsA();
        std::vector<std::string> result;
        
        if (envBlock) {
            char* current = envBlock;
            while (*current) {
                result.push_back(std::string(current));
                current += strlen(current) + 1;
            }
            FreeEnvironmentStringsA(envBlock);
        }
        return result;
    }
}

// ======================== Process ========================
namespace process {
    ProcessResult run(const std::string& command,
                      const std::vector<std::string>& args,
                      const std::string& cwd) {
        (void)cwd;
        ProcessResult result{};
        
        // Build command line
        std::string cmdLine = command;
        for (const auto& arg : args) {
            cmdLine += " \"" + arg + "\"";
        }
        
        // Create pipes for stdout/stderr
        SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
        HANDLE hRead, hWrite;
        CreatePipe(&hRead, &hWrite, &sa, 0);
        
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
        si.hStdOutput = hWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;
        
        if (CreateProcessA(nullptr, const_cast<char*>(cmdLine.c_str()), 
                          nullptr, nullptr, TRUE, 0, nullptr, 
                          cwd.empty() ? nullptr : cwd.c_str(),
                          &si, &pi)) {
            CloseHandle(hWrite);
            
            // Read stdout
            char buffer[4096];
            DWORD bytesRead;
            std::string stdoutStr;
            while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                stdoutStr += buffer;
            }
            
            CloseHandle(hRead);
            WaitForSingleObject(pi.hProcess, INFINITE);
            
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            result.exit_code = exitCode;
            result.stdout_data = stdoutStr;
            result.success = (exitCode == 0);
            
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        
        return result;
    }

    bool isAvailable(const std::string& tool) {
        std::string result = findInPath(tool);
        return !result.empty();
    }

    std::string findInPath(const std::string& name) {
        auto pathOpt = env::get("PATH");
        if (!pathOpt) return "";
        
        std::stringstream ss(*pathOpt);
        std::string dir;
        while (std::getline(ss, dir, ';')) {
            std::string candidate = path::join(dir, name + ".exe");
            if (fs::exists(candidate)) {
                return candidate;
            }
        }
        return "";
    }
}

// ======================== Terminal ========================
namespace terminal {
    bool supportsColor() {
        // Windows 10+ supports ANSI colors
        return true;
    }

    int getWidth() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            return csbi.srWindow.Right - csbi.srWindow.Left + 1;
        }
        return 80;
    }

    int getHeight() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
        return 24;
    }

    void clear() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        DWORD count;
        FillConsoleOutputCharacterA(GetStdHandle(STD_OUTPUT_HANDLE), ' ', 
                                    csbi.dwSize.X * csbi.dwSize.Y, 
                                    csbi.dwCursorPosition, &count);
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), csbi.dwCursorPosition);
    }

    void setColor(bool) {}

    std::string colorize(const std::string& text, int color_code) {
        // Windows Console API for colors
        return text;  // Simplified - would need console handle management
    }
}

// ======================== Threads ========================
namespace thread {
    void spawn(Task task) {
        std::thread t(task);
        t.detach();
    }

    void join(void*) {}
    
    void sleepMilliseconds(uint32_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void sleepMicroseconds(uint32_t us) {
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }

    struct Mutex {
        SRWLOCK lock;
        Mutex() { InitializeSRWLock(&lock); }
        ~Mutex() {}
    };

    Mutex* mutexCreate() { return new Mutex(); }
    void mutexLock(Mutex* m) { AcquireSRWLockExclusive(&m->lock); }
    void mutexUnlock(Mutex* m) { ReleaseSRWLockExclusive(&m->lock); }
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
        localtime_s(&tm, &time_t);
        
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
    // Windows networking uses Winsock
    static bool ws2_init = []() {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    }();

    TCPSocket tcpOpen(const std::string& host, uint16_t port) {
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
        if (sock.fd == INVALID_SOCKET) {
            sock.valid = false;
            freeaddrinfo(res);
            return sock;
        }
        
        if (connect(sock.fd, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sock.fd);
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
            closesocket(sock.fd);
            sock.valid = false;
        }
    }

    int tcpSend(TCPSocket& sock, const uint8_t* data, int len) {
        if (!sock.valid) return SOCKET_ERROR;
        return send(sock.fd, reinterpret_cast<const char*>(data), len, 0);
    }

    int tcpReceive(TCPSocket& sock, uint8_t* buf, int maxlen) {
        if (!sock.valid) return SOCKET_ERROR;
        return recv(sock.fd, reinterpret_cast<char*>(buf), maxlen, 0);
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
        HMODULE handle;
    };

    Library* open(const std::string& path) {
        HMODULE handle = LoadLibraryA(path.c_str());
        if (!handle) return nullptr;
        return new Library{handle};
    }

    void close(Library* lib) {
        if (lib) {
            FreeLibrary(lib->handle);
            delete lib;
        }
    }

    void* getSymbol(Library* lib, const std::string& name) {
        if (!lib) return nullptr;
        return (void*)GetProcAddress(lib->handle, name.c_str());
    }

    std::string getLastError() {
        DWORD error = GetLastError();
        char* message = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPSTR)&message, 0, nullptr);
        std::string result(message ? message : "Unknown error");
        if (message) LocalFree(message);
        return result;
    }
}

// ======================== Platform Capabilities ========================
namespace capabilities {
    bool hasEpoll() { return false; }
    bool hasKqueue() { return false; }
    bool hasIOUring() { return false; }
    bool hasInotify() { return false; }
    bool hasFSEvents() { return false; }
    bool hasReadDirChanges() { return true; }
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
