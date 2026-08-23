// EnginotechC++ — Web Hello World
// Demonstrates function calls and string operations

fn greet(name: string) -> string {
    return "Hello, " + name + "!";
}

fn add(a: int, b: int) -> int {
    return a + b;
}

fn main() {
    let message = greet("World");
    print(message);
    
    let sum = add(10, 20);
    print("10 + 20 = " + str(sum));
    
    print("\nWeb example complete!");
}
