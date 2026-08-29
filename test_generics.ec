// Generics test file - syntax validation

// Generic function
fn identity<T>(x: T) -> T { return x; }

// Generic function with multiple type params
fn pair<A, B>(first: A, second: B) -> Pair<A, B> { 
    return Pair { first, second }; 
}

// Generic struct
struct Vec<T> {
    data: array<T>;
    len: int;
}

// Generic enum
enum Option<T> {
    Some(T),
    None
}

enum Result<T, E> {
    Ok(T),
    Err(E)
}

// Generic impl
impl<T> Vec<T> {
    fn new() -> Vec<T> { return Vec { data: [], len: 0 }; }
    fn push(self, item: T) -> void { }
    fn pop(self) -> Option<T> { return Option.None; }
    fn get(self, index: int) -> Option<T> { return Option.None; }
}

impl<T, E> Result<T, E> {
    fn unwrap(self) -> T { 
        match self {
            Result.Ok(val) => val,
            Result.Err(_) => panic!("unwrap failed")
        }
    }
}

struct Pair<A, B> {
    first: A;
    second: B;
}

fn main() {
    // Type inference from arguments
    let x = identity(42);        // T = int
    let y = identity("hello");   // T = string
    let z = identity(3.14);      // T = float64
    
    // Generic struct usage
    let vec: Vec<int> = Vec<int>::new();
    vec.push(1);
    vec.push(2);
    
    // Generic enum usage
    let opt: Option<string> = Option.Some("hello");
    let none: Option<int> = Option.None;
    
    let res: Result<int, string> = Result.Ok(42);
    let err: Result<int, string> = Result.Err("error");
    
    // Pair
    let p = pair(1, "one");
    
    print("Generics syntax test passed!");
}