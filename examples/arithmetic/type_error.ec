// Type Error Test - Adding int and string should fail
fn main() {
    let a = 10;
    let name = "hello";
    
    // This should fail - int + string is invalid arithmetic
    let answer = a + name;
    print(answer);
}
