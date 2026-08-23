// EnginotechC++ — Web Example: Calculator
// Demonstrates function calls and types

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
    return a / b;
}

fn isEven(n: int) -> bool {
    if (n % 2 == 0) {
        return true;
    } else {
        return false;
    }
}

fn getGreeting(name: string) -> string {
    return "Hello, " + name + "!";
}

fn main() {
    print("=== Calculator Demo ===\n");
    
    let a = 10;
    let b = 5;
    
    print("add(" + str(a) + ", " + str(b) + ") = " + str(add(a, b)));
    print("subtract(" + str(a) + ", " + str(b) + ") = " + str(subtract(a, b)));
    print("multiply(" + str(a) + ", " + str(b) + ") = " + str(multiply(a, b)));
    print("divide(" + str(a) + ", " + str(b) + ") = " + str(divide(a, b)));
    
    print("\nEven/Odd Check:");
    let num = 7;
    if (isEven(num)) {
        print(str(num) + " is even");
    } else {
        print(str(num) + " is odd");
    }
    
    print("\n" + getGreeting("EC Web"));
}
