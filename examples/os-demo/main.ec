// EnginotechC++ — OS & Hardware Chat Demo (Complete)
// Fitur baru: OS builtins + Serial + Helpers

fn main() -> int {
    // === OS Builtins ===
    
    // run_command: jalankan perintah shell
    print("=== OS Commands ===");
    let code = run_command("echo Hello dari Shell EC");
    print("run_command exit: ", code);
    
    let date = exec_output("date '+%Y-%m-%d %H:%M:%S'");
    print("Tanggal: ", date);
    
    let hostname = exec_output("hostname");
    print("Hostname: ", hostname);
    
    // open_app
    let opened = open_app("no-such-app-xyz");
    print("Open app berhasil: ", opened);
    
    // === Environment & Path ===
    print("\n=== Environment & Path ===");
    
    let home = env_get("HOME");
    print("Home dir: ", home);
    
    let cwd = cwd();
    print("Working dir: ", cwd);
    
    let joined = path_join(cwd, "docs");
    print("Path join(cwd, 'docs'): ", joined);
    
    // === Timing Helper ===
    print("\n=== Timing (sleep 100ms) ===");
    sleep(100);
    print("Dormi 100ms selesai\n");
    
    // === Serial Hardware Chat ===
    print("=== Serial Hardware Chat ===");
    
    // Coba port fake dulu
    let bad = serial_open("/dev/ttyFAKE-NOEXIST", 115200);
    print("Port fake valid: ", bad);
    
    // Buka PTY untuk demo
    let port = "/dev/pts/0";
    let ok = serial_open(port, 115200);
    print("serial_open(", port, "): ", ok);
    
    if ok {
        // Pola komunikasi dengan Arduino/hardware
        serial_write("AT\r\n");       // perintah AT ke Arduino
        sleep(100);                 // tunggu respons
        
        let response = serial_read_line();
        print("Response: ", response);
        
        serial_write("LED ON\n");
        sleep(50);
        
        serial_close();
        print("Serial ditutup\n");
    }
    
    return 0;
}
