// Smoke 12 — OS Builtins (run_command, exec_output, open_app)
fn main() -> int {
    let code = run_command("echo hello-12");
    print(code);

    let out = exec_output("printf abc123");
    print(out);

    let missing = open_app("no-such-app-xyz-42");
    print(missing);
    return 0;
}
