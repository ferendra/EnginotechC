// M1 Type Inference Test Cases
// Tests that type inference works correctly

// === Variable Type Inference ===
// These should auto-infer types

let int_x = 42           // x is int (inferred from literal)
let float_pi = 3.14      // pi is float (inferred from literal)
let str_name = "EC"      // name is string (inferred from literal)
let bool_active = true   // active is bool (inferred from literal)

// === Function Return Type Inference ===
// Functions without explicit -> type should work

fn add(a, b) {
    return a + b;
    // return type 'int' is inferred from the return statement
}

fn square(x) {
    return x * x;
    // return type inferred from body expression
}

// === Main Function ===

fn main() {
    // Test inference results
    let result1 = add(2, 3);  // result should be int (inferred from add's return)
    let result2 = square(5);  // result should be int (inferred from square's return)
    
    // Test basic operations with inferred types
    let x = 10;
    let y = 5;
    let sum = x + y;  // should be int (inferred from x and y being int)
    let product = x * y;  // should be int
    
    // Test boolean operations
    let is_true = true;
    let is_false = false;
    let and_result = is_true && is_false;  // should be bool
    let or_result = is_true || is_false;   // should be bool
    
    // Test comparisons
    let eq = (x == y);  // should be bool
    let neq = (x != y);  // should be bool
    
    // Print results
    print("M1 Type Inference Test");
    print("x = " + str(x));
    print("sum = " + str(sum));
    print("product = " + str(product));
    print("and_result = " + str(and_result));
    print("eq = " + str(eq));
}