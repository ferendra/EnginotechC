// Test suite for basic operations
// Run with: engc test tests/

test "addition" {
    assert(add(2, 3) == 5);
    assert(add(-1, 1) == 0);
    assert(add(0, 0) == 0);
}

test "subtraction" {
    assert(sub(5, 3) == 2);
    assert(sub(0, 5) == -5);
}

test "multiplication" {
    assert(mul(3, 4) == 12);
    assert(mul(0, 100) == 0);
}

test "factorial" {
    assert(factorial(0) == 1);
    assert(factorial(1) == 1);
    assert(factorial(5) == 120);
    assert(factorial(10) == 3628800);
}

fn add(a: int, b: int) -> int {
    return a + b;
}

fn sub(a: int, b: int) -> int {
    return a - b;
}

fn mul(a: int, b: int) -> int {
    return a * b;
}

fn factorial(n: int) -> int {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

fn main() {
    print("Running tests...");
    // Tests are run by the test harness
}
