// Benchmark: Recursive Fibonacci
fn fib(n: int) -> int {
    if n <= 1 {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

fn main() {
    let result = fib(30);
    print("fib(30) = " + str(result));
    assert(result == 832040);
    print("recursive_benchmark OK");
}
