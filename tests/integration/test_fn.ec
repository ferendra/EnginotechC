// Integration test: functions — params, returns, recursion
fn add(a: int, b: int) -> int {
    return a + b;
}

fn fib(n: int) -> int {
    if n < 2 {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

fn main() {
    assert(add(2, 3) == 5);
    assert(fib(10) == 55);
    print("test_fn OK");
}
