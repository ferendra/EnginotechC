// EnginotechC++ Platform Abstraction Demo
// This file demonstrates the cross-platform abstraction layer
// Supported platforms: Linux, FreeBSD, OpenBSD, NetBSD, macOS, Windows (MinGW)

import platform;
import io;

fn main() {
    // System information
    let sys = platform.getSystemInfo();
    io.println("=== EnginotechC++ Platform Abstraction Demo ===");
    io.println("");
    
    // OS Detection
    io.print("OS: ");
    io.println(sys.os);
    io.print("Version: ");
    io.println(sys.os_version);
    io.print("Architecture: ");
    io.println(sys.arch);
    io.print("Hostname: ");
    io.println(sys.hostname);
    io.print("CPU Cores: ");
    io.println(sys.cpu_count);
    io.println("");
    
    // Memory info
    io.println("--- Memory ---");
    let memTotal = sys.memory.total_bytes;
    let memFree = sys.memory.free_bytes;
    io.print("Total Memory: ");
    io.print((memTotal / 1024 / 1024 / 1024));
    io.println(" GB");
    io.print("Free Memory: ");
    io.print((memFree / 1024 / 1024 / 1024));
    io.println(" GB");
    io.println("");
    
    // Platform capabilities
    io.println("--- Platform Capabilities ---");
    if (platform.capabilities.hasEpoll()) {
        io.println("  [x] epoll (Linux event system)");
    } else {
        io.println("  [ ] epoll (Linux event system)");
    }
    
    if (platform.capabilities.hasKqueue()) {
        io.println("  [x] kqueue (BSD/macOS event system)");
    } else {
        io.println("  [ ] kqueue (BSD/macOS event system)");
    }
    io.println("");
    
    // Current directory
    io.println("--- Paths ---");
    io.print("Current Dir: ");
    io.println(platform.path.getCurrentDir());
    io.print("User Home: ");
    io.println(platform.path.getUserHome());
    io.println("");
    
    // Time
    io.println("--- Time ---");
    let now = platform.time.nowMillis();
    io.print("Current time (ms): ");
    io.println(now);
    io.println("");
    
    // Terminal
    io.println("--- Terminal ---");
    io.print("Supports Color: ");
    io.println(platform.terminal.supportsColor());
    io.print("Width: ");
    io.println(platform.terminal.getWidth());
    io.println("");
    
    // Environment
    io.println("--- Environment ---");
    if let path = platform.env.get("PATH") {
        io.print("PATH: ");
        io.println(path);
    }
    io.println("");
    
    // Filesystem
    io.println("--- Filesystem ---");
    if platform.fs.exists("CMakeLists.txt") {
        io.println("[x] CMakeLists.txt exists");
    }
    if platform.fs.isDirectory(".") {
        io.println("[x] Current directory is valid");
    }
    io.println("");
    
    io.println("=== Demo Complete ===");
}
