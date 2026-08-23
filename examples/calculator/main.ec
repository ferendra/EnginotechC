// Calculator Example - EnginotechC++
fn add(a: int, b: int) -> int {
    return a + b;
}

fn subtract(a: int, b: int) -> int {
    return a - b;
}

fn multiply(a: int, b: int) -> int {
    return a * b;
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
    
    print("Calculator Demo");
    print("a = " + str(a));
    print("b = " + str(b));
    print("---");
    
    print("add(" + str(a) + ", " + str(b) + ") = " + str(add(a, b)));
    print("subtract(" + str(a) + ", " + str(b) + ") = " + str(subtract(a, b)));
    print("multiply(" + str(a) + ", " + str(b) + ") = " + str(multiply(a, b)));
    print("divide(" + str(a) + ", " + str(b) + ") = " + str(divide(a, b)));
    
    // Test division by zero
    print("divide(" + str(a) + ", 0) = " + str(divide(a, 0)));
}
