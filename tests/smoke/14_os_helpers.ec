// Smoke 14 — OS Helper Functions (sleep, env_get, cwd, path_join)
fn main() -> int {
    let h = env_get("HOME");
    print(h);

    let c = cwd();
    print(c);

    let p = path_join("/", "usr/local");
    print(p);

    sleep(10);
    print("slept\n");
    return 0;
}
