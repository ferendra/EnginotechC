// Smoke 13 — Serial Hardware Chat
fn main() -> int {
    let bad = serial_open("/dev/ttyFAKE-NOEXIST", 115200);
    print(bad);

    let opened = serial_open("/dev/pts/3", 115200);
    print(opened);

    let w = serial_write("ping dari EC\n");
    print(w);

    let line = serial_read_line();
    print(line);

    serial_close();
    return 0;
}
