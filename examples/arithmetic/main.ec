// Arithmetic Variables Example - EnginotechC++
// Demonstrates all arithmetic operations on variables

fn main() {
    // Integer arithmetic
    let a = 10;
    let b = 3;
    
    print("=== Integer Arithmetic ===");
    print("a = " + str(a));
    print("b = " + str(b));
    print("");
    
    // Addition
    let sum = a + b;
    print("a + b = " + str(sum));
    
    // Subtraction
    let diff = a - b;
    print("a - b = " + str(diff));
    
    // Multiplication
    let prod = a * b;
    print("a * b = " + str(prod));
    
    // Division
    let quot = a / b;
    print("a / b = " + str(quot));
    
    // Modulo
    let rem = a % b;
    print("a % b = " + str(rem));
    print("");
    
    // Float arithmetic
    let x = 10.5;
    let y = 3.2;
    
    print("=== Float Arithmetic ===");
    print("x = " + str(x));
    print("y = " + str(y));
    print("");
    
    let fsum = x + y;
    print("x + y = " + str(fsum));
    
    let fdiff = x - y;
    print("x - y = " + str(fdiff));
    
    let fprod = x * y;
    print("x * y = " + str(fprod));
    
    let fquot = x / y;
    print("x / y = " + str(fquot));
    print("");
    
    // Mixed arithmetic (int + float)
    let mixed = a + y;
    print("=== Mixed Arithmetic ===");
    print("a + y = " + str(mixed));
    
    // Unary operators
    let neg = -a;
    print("Negation: -a = " + str(neg));
    
    let pos = +b;
    print("Unary plus: +b = " + str(pos));
    
    // Complex expressions - step by step
    let step1 = a + b;
    let step2 = step1 * 2;
    let step3 = step2 - 5;
    print("Complex: ((a + b) * 2) - 5 = " + str(step3));
    
    // Using arithmetic in conditions
    let grade = 85;
    if grade >= 90 {
        print("Grade: A");
    } else if grade >= 80 {
        print("Grade: B");
    } else {
        print("Grade: C");
    }
}
