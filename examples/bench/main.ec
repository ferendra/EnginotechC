// Benchmark: recursive fib + tight loop sum
fn fib(n: int) -> int {
    if n < 2 {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

fn main() {
    let r = fib(32);
    print("fib(32) =", r);

    let mut total = 0;
    for i in 0..100000000 {
        total += i;
    }
    print("loop sum =", total);
}
