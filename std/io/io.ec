// I/O Module for EnginotechC++

fn print(s: string) -> void { }
fn println(s: string) -> void { }
fn printf(format: string, args: ...) -> void { }

fn input() -> string { return ""; }
fn read_line() -> string { return ""; }
fn read_all() -> string { return ""; }
fn read_int() -> int { return 0; }
fn read_float() -> float64 { return 0.0; }
fn read_bool() -> bool { return false; }

fn file_exists(path: string) -> bool { return false; }
fn read_file(path: string) -> string { return ""; }
fn write_file(path: string, content: string) -> bool { return false; }
fn append_file(path: string, content: string) -> bool { return false; }

fn range(n: int) -> array<int> { return []; }