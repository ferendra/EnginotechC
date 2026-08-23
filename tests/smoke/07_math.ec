// Smoke 07 — math & conversion builtins
fn main() {
    let n = int("42");
    let f = float("2.5");
    print(n * 2);
    print(f + 0.5);
    print(abs(-7));
    print(abs(-2.5));
    print(min(3, 9));
    print(max(3, 9));
    print(sqrt(16.0));
    print(pow(2.0, 10.0));
    print(floor(3.9));
    print(ceil(3.1));
}
