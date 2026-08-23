// Smoke 02 — arithmetic, functions, if/else branch
fn add(a: int, b: int) -> int {
    return a + b;
}

fn divide(a: int, b: int) -> int {
    if b == 0 {
        print("Error: Division by zero!");
        return 0;
    }
    return a / b;
}

fn main() {
    let a = 10;
    let b = 5;
    print("add = " + str(add(a, b)));
    print("div = " + str(divide(a, b)));
    divide(a, 0);
}
